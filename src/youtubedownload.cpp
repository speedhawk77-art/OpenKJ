// SPDX-License-Identifier: GPL-3.0-or-later
#include "youtubedownload.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QUrl>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrent>
#include <gst/pbutils/pbutils.h>
#include <limits>
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

// A download can start ffmpeg and Deno. Cancel/shutdown must stop those too.
class DownloadProcess : public QProcess {
public:
    explicit DownloadProcess(QObject *parent) : QProcess(parent) {}
    ~DownloadProcess() override { stopTree(); waitForFinished(3000); releaseTree(); }
    bool attachTree() {
#ifdef Q_OS_WIN
        m_job = CreateJobObjectW(nullptr, nullptr);
        if (!m_job) return false;
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION info{};
        info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (!SetInformationJobObject(m_job, JobObjectExtendedLimitInformation, &info, sizeof(info))) return false;
        HANDLE process = OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, static_cast<DWORD>(processId()));
        if (!process) return false;
        bool ok = AssignProcessToJobObject(m_job, process) != FALSE;
        CloseHandle(process);
        return ok;
#else
        m_group = processId();
        return m_group > 0;
#endif
    }
    void stopTree() {
#ifdef Q_OS_WIN
        if (m_job) TerminateJobObject(m_job, 1);
#else
        if (m_group > 0) ::kill(-static_cast<pid_t>(m_group), SIGKILL);
#endif
        if (state() != NotRunning) kill();
    }
    void releaseTree() {
#ifdef Q_OS_WIN
        if (m_job) { CloseHandle(m_job); m_job = nullptr; }
#else
        if (m_group > 0) ::kill(-static_cast<pid_t>(m_group), SIGKILL);
        m_group = 0;
#endif
    }
private:
#ifdef Q_OS_WIN
    HANDLE m_job{nullptr};
#else
    qint64 m_group{0};
    void setupChildProcess() override { ::setpgid(0, 0); }
#endif
};

QString YouTubeDownload::canonicalUrl(const QString &text) {
    const QUrl url(text.trimmed(), QUrl::StrictMode);
    if (!url.isValid() || (url.scheme() != "https" && url.scheme() != "http")
        || !url.userInfo().isEmpty() || (url.port() != -1 && url.port() != 443)) return {};
    const auto host = url.host().toLower();
    const auto parts = url.path().split('/', Qt::SkipEmptyParts);
    QString id;
    if (host == "youtu.be" && parts.size() == 1) id = parts[0];
    if (host == "youtube.com" || host == "www.youtube.com" || host == "m.youtube.com" || host == "music.youtube.com") {
        if (url.path() == "/watch") id = QUrlQuery(url).queryItemValue("v");
        else if (parts.size() == 2 && (parts[0] == "shorts" || parts[0] == "live")) id = parts[1];
    }
    if (!QRegularExpression("^[A-Za-z0-9_-]{11}$").match(id).hasMatch()) return {};
    return "https://www.youtube.com/watch?v=" + id;
}

YouTubeDownload::YouTubeDownload(QObject *parent) : QObject(parent), m_process(new DownloadProcess(this)) {
    m_deadline.setSingleShot(true);
    m_deadline.setInterval(15 * 60 * 1000);
    connect(&m_deadline, &QTimer::timeout, this, [this] { m_timedOut = true; cancel(); });
    connect(m_process.get(), &QProcess::started, this, [this] {
        if (!m_process->attachTree()) {
            m_errorTail = "Unable to manage the downloader's child processes. The download was stopped.";
            m_process->kill();
        } else if (m_cancelled->load()) m_process->stopTree();
    });
    connect(m_process.get(), &QProcess::readyReadStandardError, this, [this] {
        m_errorTail = (m_errorTail + m_process->readAllStandardError()).right(8192);
    });
    connect(m_process.get(), &QProcess::readyReadStandardOutput, this, [this] {
        m_outputTail += m_process->readAllStandardOutput();
        int end;
        while ((end = m_outputTail.indexOf('\n')) >= 0) {
            auto line = m_outputTail.left(end).trimmed(); m_outputTail.remove(0, end + 1);
            if (line.startsWith("OPENKJ_PROGRESS:")) emit progress(QString::fromUtf8(line.mid(16)).left(200));
        }
        m_outputTail = m_outputTail.right(4096);
    });
    connect(m_process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this](int code, QProcess::ExitStatus status) { processFinished(code, static_cast<int>(status)); });
    connect(m_process.get(), &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart && m_busy) fail(tr("Could not start yt-dlp: %1").arg(m_process->errorString()));
    });
    connect(&m_watcher, &QFutureWatcher<Result>::finished, this, [this] {
        const auto result = m_watcher.result();
        reset();
        if (result.cancelled) emit cancelled();
        else if (!result.error.isEmpty()) emit failed(result.error);
        else emit ready(result.path, result.title, result.videoId, result.durationMs);
    });
}

YouTubeDownload::~YouTubeDownload() {
    if (m_cancelled) m_cancelled->store(true);
    m_process->disconnect(this);
    m_process->stopTree();
    m_process->waitForFinished(3000);
    m_process->releaseTree();
    // The verification task owns its temporary directory and cancellation flag.
    // It never accesses this QObject and can safely finish after the dialog closes.
}

bool YouTubeDownload::start(const Request &request) {
    if (m_busy) return false;
    auto reject = [this](const QString &error) { emit failed(error); return false; };
    const auto url = canonicalUrl(request.url);
    if (url.isEmpty()) return reject(tr("Paste a link to one YouTube video, such as https://youtu.be/VIDEO_ID."));
    for (const auto &tool : {request.downloader, request.ffmpeg, request.deno}) {
        const QFileInfo info(tool);
        if (tool.isEmpty() || !info.isAbsolute() || !info.isFile() || !info.isExecutable())
            return reject(tr("Select the installed yt-dlp, ffmpeg and Deno programs in One-time setup."));
#ifdef Q_OS_WIN
        if (info.suffix().compare("exe", Qt::CaseInsensitive) != 0)
            return reject(tr("Choose the .exe program for each helper, not a script or shortcut."));
#endif
    }
    const QFileInfo destination(request.destination);
    if (!destination.isAbsolute() || !destination.isDir()) return reject(tr("Choose an existing folder for the completed videos."));
    const auto cache = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/youtube-downloads";
    if (!QDir().mkpath(cache)) return reject(tr("Could not create a temporary download folder."));
    m_temporary = std::make_shared<QTemporaryDir>(cache + "/download-XXXXXX");
    if (!m_temporary->isValid()) return reject(tr("Could not create a temporary download folder."));
    m_request = request; m_request.url = url;
    m_request.destination = destination.canonicalFilePath();
    m_cancelled = std::make_shared<std::atomic_bool>(false);
    m_busy = true; m_timedOut = false; m_errorTail.clear(); m_outputTail.clear();
    const auto metadata = m_temporary->filePath("result.json");
    QStringList args{
        "--ignore-config", "--no-plugin-dirs", "--no-playlist", "--no-simulate", "--no-overwrites",
        "--no-live-from-start", "--match-filter", "!is_live & !is_upcoming",
        "--socket-timeout", "20", "--retries", "2", "--fragment-retries", "2",
        "--format", "bv[ext=mp4][vcodec^=avc1][height<=1080]+ba[ext=m4a]/b[ext=mp4][vcodec^=avc1][acodec!=none][height<=1080]",
        "--merge-output-format", "mp4", "--ffmpeg-location", request.ffmpeg,
        "--no-js-runtimes", "--js-runtimes", "deno:" + request.deno, "--no-remote-components",
        "--output", m_temporary->filePath("video.%(ext)s"),
        "--print-to-file", "after_move:%(.{id,title,filepath})j", metadata,
        "--newline", "--progress", "--progress-template", "download:OPENKJ_PROGRESS:%(progress._percent_str)s of %(progress._total_bytes_str)s",
        "--", url
    };
    // Output and metadata paths are yt-dlp templates too. Escape literal percent signs.
    auto escapePath = [](QString path) { return path.replace('%', "%%"); };
    args[args.indexOf("--output") + 1] = escapePath(m_temporary->path()) + "/video.%(ext)s";
    args[args.indexOf("--print-to-file") + 2] = escapePath(metadata);
    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8"); env.insert("PYTHONUTF8", "1");
    m_process->setProcessEnvironment(env);
    m_process->setWorkingDirectory(m_temporary->path());
    m_deadline.start();
    emit progress(tr("Downloading video and audio…"));
    m_process->start(request.downloader, args);
    return true;
}

void YouTubeDownload::cancel() {
    if (!m_busy) return;
    m_cancelled->store(true);
    emit progress(tr("Cancelling…"));
    if (m_process->state() != QProcess::NotRunning) m_process->stopTree();
}

void YouTubeDownload::reset() {
    m_deadline.stop(); m_process->releaseTree();
    m_busy = false; m_temporary.reset();
}

void YouTubeDownload::fail(const QString &message) {
    reset(); emit failed(message);
}

void YouTubeDownload::processFinished(int code, int status) {
    if (!m_busy) return;
    m_process->releaseTree();
    m_errorTail = (m_errorTail + m_process->readAllStandardError()).right(8192);
    if (m_timedOut) { fail(tr("The download exceeded 15 minutes and was stopped. Try again on a faster connection.")); return; }
    if (m_cancelled->load()) { reset(); emit cancelled(); return; }
    if (code != 0 || status != static_cast<int>(QProcess::NormalExit)) {
        fail(tr("Download failed. Check your connection and helper versions.\n%1").arg(QString::fromUtf8(m_errorTail.right(1200)).trimmed())); return;
    }
    QFile metadata(m_temporary->filePath("result.json"));
    if (!metadata.open(QIODevice::ReadOnly) || metadata.size() > 65536) { fail(tr("The downloader did not report a completed video.")); return; }
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(metadata.readAll().trimmed(), &error);
    const auto info = document.object();
    const auto id = QUrlQuery(QUrl(m_request.url)).queryItemValue("v");
    const QFileInfo file(info.value("filepath").toString());
    const auto expected = QFileInfo(m_temporary->filePath("video.mp4")).canonicalFilePath();
    if (error.error != QJsonParseError::NoError || !document.isObject() || info.value("id").toString() != id
        || expected.isEmpty() || file.isSymLink() || file.canonicalFilePath() != expected || file.size() < 32
        || info.value("title").toString().trimmed().isEmpty()) {
        fail(tr("The downloaded video or its completion information is invalid. Nothing was added to the library.")); return;
    }
    emit progress(tr("Checking the completed video and saving it…"));
    const auto temporary = m_temporary;
    const auto cancelled = m_cancelled;
    const auto destination = m_request.destination;
    const auto title = info.value("title").toString().trimmed();
    m_watcher.setFuture(QtConcurrent::run([temporary, cancelled, destination, title, id, expected] {
        return verifyAndSave(expected, destination, title, id, cancelled);
    }));
}

YouTubeDownload::Result YouTubeDownload::verifyAndSave(const QString &source, const QString &destination,
        const QString &title, const QString &videoId, const std::shared_ptr<std::atomic_bool> &cancelled) {
    Result result; result.title = title; result.videoId = videoId;
    if (cancelled->load()) { result.cancelled = true; return result; }
    GError *error = nullptr;
    auto *discoverer = gst_discoverer_new(5 * GST_SECOND, &error);
    auto *info = discoverer ? gst_discoverer_discover_uri(discoverer, QUrl::fromLocalFile(source).toEncoded().constData(), &error) : nullptr;
    bool valid = info && gst_discoverer_info_get_result(info) == GST_DISCOVERER_OK;
    auto *video = info ? gst_discoverer_info_get_video_streams(info) : nullptr;
    auto *audio = info ? gst_discoverer_info_get_audio_streams(info) : nullptr;
    valid = valid && video && audio;
    const auto duration = info ? gst_discoverer_info_get_duration(info) : GST_CLOCK_TIME_NONE;
    valid = valid && GST_CLOCK_TIME_IS_VALID(duration) && duration > 0
        && duration / GST_MSECOND <= static_cast<guint64>(std::numeric_limits<int>::max());
    if (audio) gst_discoverer_stream_info_list_free(audio);
    if (video) gst_discoverer_stream_info_list_free(video);
    if (info) gst_discoverer_info_unref(info);
    if (discoverer) g_object_unref(discoverer);
    if (error) g_error_free(error);
    if (cancelled->load()) { result.cancelled = true; return result; }
    if (!valid) { result.error = tr("OpenKJ could not verify both audio and video in this download. On the laptop, check Help → Optional H.264 codec…, enable it if you agree to the terms, then restart OpenKJ and retry. If it is already enabled, the file may be damaged or unsupported. Nothing was added to the library."); return result; }
    result.durationMs = static_cast<int>(duration / GST_MSECOND);
    QTemporaryFile output(QDir(destination).filePath(".openkj-import-XXXXXX"));
    QFile input(source);
    if (!input.open(QIODevice::ReadOnly) || !output.open()) { result.error = tr("Could not save the video in the selected folder."); return result; }
    while (!input.atEnd()) {
        if (cancelled->load()) { result.cancelled = true; return result; }
        const auto bytes = input.read(1024 * 1024);
        if (bytes.isEmpty() || output.write(bytes) != bytes.size()) { result.error = tr("Could not finish saving the video. Check free disk space."); return result; }
    }
    if (!output.flush()) { result.error = tr("Could not finish saving the video. Check free disk space."); return result; }
    output.close(); input.close();
    QString filename = title;
    filename.replace(QRegularExpression("[\\x00-\\x1f<>:\"/\\\\|?*]"), "_");
    filename = filename.left(100).trimmed();
    while (filename.endsWith('.') || filename.endsWith(' ')) filename.chop(1);
    if (!filename.isEmpty() && filename.at(filename.size() - 1).isHighSurrogate()) filename.chop(1);
    if (filename.isEmpty()) filename = "YouTube video";
    for (int i = 0; i < 1000; ++i) {
        if (cancelled->load()) { result.cancelled = true; return result; }
        const auto suffix = i == 0 ? QString() : QString(" (%1)").arg(i + 1);
        const auto path = QDir(destination).filePath(filename + " [YT-" + videoId + "]" + suffix + ".mp4");
        // Same-directory rename publishes a complete file and never overwrites an existing song.
        // Use this temporary file's engine: Qt 5 close() retains its Windows handle.
        if (output.rename(path)) { output.setAutoRemove(false); result.path = QDir::fromNativeSeparators(QFileInfo(path).absoluteFilePath()); return result; }
        if (!QFileInfo::exists(path)) { result.error = tr("Could not save the video. Check the folder permissions and path length."); return result; }
    }
    result.error = tr("Too many existing copies of this video in the selected folder."); return result;
}
