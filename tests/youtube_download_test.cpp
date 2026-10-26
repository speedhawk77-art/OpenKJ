// SPDX-License-Identifier: GPL-3.0-or-later
// Real controller, file validation, dialog and library import; synthetic helper/media only.
#include "youtubedownload.h"
#include "remotecontrolserver.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHostAddress>
#include <QUrlQuery>
#include "dlgyoutubedownload.h"
#include "models/tablemodelkaraokesongs.h"
#include "idledetect.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <gst/gst.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <functional>
#include <iostream>

IdleDetect *filter = nullptr;
struct YouTubeDownloadTestAccess {
    static void timeout(YouTubeDownload &job, int milliseconds) { job.m_deadline.setInterval(milliseconds); }
};

static void writeFile(const QString &path, const QByteArray &data) {
    QFile f(path); if (!f.open(QIODevice::WriteOnly) || f.write(data) != data.size()) throw std::runtime_error("Fixture write failed");
}
static QByteArray readFile(const QString &path) {
    QFile f(path); if (!f.open(QIODevice::ReadOnly)) return {}; return f.readAll();
}
static void spin(int milliseconds) {
    QElapsedTimer timer; timer.start();
    while (timer.elapsed() < milliseconds) { QCoreApplication::processEvents(); QThread::msleep(5); }
}
static bool waitFor(const std::function<bool()> &condition, int milliseconds = 15000) {
    QElapsedTimer timer; timer.start();
    while (!condition() && timer.elapsed() < milliseconds) spin(5);
    return condition();
}

static int fakeDownloader(const QStringList &args) {
    auto value = [&args](const QString &name) { const auto i = args.indexOf(name); return i < 0 ? QString() : args.value(i + 1); };
    auto output = value("--output").replace("%(ext)s", "mp4").replace("%%", "%");
    auto metadata = args.value(args.indexOf("--print-to-file") + 2).replace("%%", "%");
    const auto id = args.last().right(11);
    const auto mode = id.toInt();
    if (mode == 2) { std::cerr << "Synthetic network failure"; return 7; }
    if (mode == 9) {
        QProcess child;
        child.start(QCoreApplication::applicationFilePath(), {"--fake-child"});
        if (!child.waitForStarted(3000)) return 8;
        std::cout << "OPENKJ_PROGRESS:child-started\n" << std::flush;
        QThread::sleep(20); return 0;
    }
    const auto base = QCoreApplication::applicationDirPath();
    writeFile(output, mode == 3 ? QByteArray(128, 'x') : readFile(base + (mode == 4 ? "/youtube-silent.mp4" : "/youtube-fixture.mp4")));
    if (mode == 8) return 0;
    if (mode == 7) { writeFile(metadata, "{invalid"); return 0; }
    QJsonObject info{{"id", mode == 6 ? "wrongID0000" : id}, {"title", "Test/Artist: Blue \"Karaoke\" é"},
                     {"filepath", mode == 5 ? base + "/youtube-fixture.mp4" : output}};
    writeFile(metadata, QJsonDocument(info).toJson(QJsonDocument::Compact));
    // In a folder containing a percent sign, every literal percent in templates must be doubled.
    writeFile(QDir(QFileInfo(output).absolutePath()).filePath("arguments.json"), QJsonDocument(QJsonArray::fromStringList(args)).toJson());
    std::cout << "unstructured helper output\nOPENKJ_PROGRESS:100%\n" << std::flush;
    return 0;
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    const auto args = app.arguments();
    if (args.contains("--ignore-config")) return fakeDownloader(args);
    if (args.contains("--fake-child")) {
        QThread::sleep(3);
        writeFile(QCoreApplication::applicationDirPath() + "/youtube-child-survived.txt", "bad"); return 0;
    }
    QCoreApplication::setOrganizationName("OpenKJ-YouTube-Tests");
    QCoreApplication::setApplicationName("Download Space % Unicode é");
    QStandardPaths::setTestModeEnabled(true);
    auto logger = spdlog::stdout_color_mt("logger"); logger->set_level(spdlog::level::err);
    gst_init(nullptr, nullptr);
    g_log_set_always_fatal(static_cast<GLogLevelFlags>(G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL));
    QJsonArray checks; bool all = true;
    auto check = [&](const QString &name, bool ok) {
        checks.append(QJsonObject{{"name", name}, {"passed", ok}}); all &= ok;
        std::cout << (ok ? "PASS " : "FAIL ") << name.toStdString() << std::endl;
    };
    try {
        const auto base = QCoreApplication::applicationDirPath();
        writeFile(base + "/youtube-fixture.mp4", QByteArray::fromBase64(readFile("tests/fixtures/youtube-av.mp4.b64")));
        writeFile(base + "/youtube-silent.mp4", QByteArray::fromBase64(readFile("community/windows/blue-h264.mp4.b64")));
        QTemporaryDir output;
        const auto program = QCoreApplication::applicationFilePath();
        auto request = [&](int mode) { return YouTubeDownload::Request{
            "https://www.youtube.com/watch?v=" + QString("%1").arg(mode, 11, 10, QChar('0')),
            program, program, program, output.path()}; };
        for (const auto &url : {"https://youtu.be/00000000001?si=x", "https://www.youtube.com/watch?v=00000000001&list=ignored", "https://m.youtube.com/shorts/00000000001", "http://youtube.com/live/00000000001"})
            check("accept_single_video_" + QString(url), YouTubeDownload::canonicalUrl(url) == "https://www.youtube.com/watch?v=00000000001");
        for (const auto &url : {"file:///etc/passwd", "https://youtube.com.evil.invalid/watch?v=00000000001", "https://user@youtube.com/watch?v=00000000001", "https://youtube.com/playlist?list=00000000001", "--exec touch", "https://youtube.com:8443/watch?v=00000000001"})
            check("reject_non_video_" + QString(url), YouTubeDownload::canonicalUrl(url).isEmpty());
        {
            YouTubeDownload job; auto bad = request(1); bad.downloader = "not-installed"; int failed = 0;
            QObject::connect(&job, &YouTubeDownload::failed, &app, [&](const QString &) { ++failed; });
            check("missing_helper_is_actionable", !job.start(bad) && failed == 1 && !job.isBusy());
        }
        QString completed; QString completedTitle; int completedDuration = 0;
        {
            YouTubeDownload job; int ready = 0, failed = 0, progress = 0, heartbeats = 0;
            QTimer heartbeat; heartbeat.setInterval(1); QObject::connect(&heartbeat, &QTimer::timeout, &app, [&] { ++heartbeats; }); heartbeat.start();
            QObject::connect(&job, &YouTubeDownload::ready, &app, [&](QString path, QString title, QString, int duration) { if (ready == 0) { completed = path; completedTitle = title; completedDuration = duration; } ++ready; });
            QObject::connect(&job, &YouTubeDownload::failed, &app, [&](QString error) { std::cerr << error.toStdString() << std::endl; ++failed; });
            QObject::connect(&job, &YouTubeDownload::progress, &app, [&](QString) { ++progress; });
            check("download_starts", job.start(request(1)));
            check("second_download_rejected_while_busy", !job.start(request(1)));
            check("completed_audio_video_verified", waitFor([&] { return !job.isBusy(); }) && ready == 1 && failed == 0 && completedDuration > 0);
            check("ui_event_loop_remains_responsive", heartbeats > 2 && progress >= 2);
            check("filename_sanitized", !completed.isEmpty() && !QFileInfo(completed).fileName().contains(':') && !QFileInfo(completed).fileName().contains('"'));
            check("downloaded_bytes_preserved", !completed.isEmpty() && readFile(completed) == readFile(base + "/youtube-fixture.mp4"));
            QString second;
            QObject::connect(&job, &YouTubeDownload::ready, &app, [&](QString path, QString, QString, int) { second = path; });
            check("same_controller_can_download_again", job.start(request(1)) && waitFor([&] { return !job.isBusy(); }) && ready == 2);
            check("existing_song_never_overwritten", !second.isEmpty() && second != completed && QFileInfo(completed).isFile() && QFileInfo(second).isFile() && readFile(completed) == readFile(base + "/youtube-fixture.mp4") && readFile(second) == readFile(base + "/youtube-fixture.mp4"));
        }
        for (int mode : {2, 3, 4, 5, 6, 7, 8}) {
            YouTubeDownload job; int ready = 0, failed = 0;
            const auto before = QDir(output.path()).entryList(QDir::Files | QDir::Hidden).size();
            QObject::connect(&job, &YouTubeDownload::ready, &app, [&](QString, QString, QString, int) { ++ready; });
            QObject::connect(&job, &YouTubeDownload::failed, &app, [&](QString) { ++failed; });
            job.start(request(mode));
            check(QString("reject_failed_or_invalid_download_%1").arg(mode), waitFor([&] { return !job.isBusy(); }) && ready == 0 && failed == 1);
            check(QString("no_partial_song_published_%1").arg(mode), before == QDir(output.path()).entryList(QDir::Files | QDir::Hidden).size());
        }
        {
            QFile::remove(base + "/youtube-child-survived.txt");
            YouTubeDownload job; int cancelled = 0, ready = 0;
            QObject::connect(&job, &YouTubeDownload::progress, &app, [&](QString message) { if (message == "child-started") job.cancel(); });
            QObject::connect(&job, &YouTubeDownload::cancelled, &app, [&] { ++cancelled; });
            QObject::connect(&job, &YouTubeDownload::ready, &app, [&](QString, QString, QString, int) { ++ready; });
            check("cancel_stops_download", job.start(request(9)) && waitFor([&] { return !job.isBusy(); }) && cancelled == 1 && ready == 0);
            spin(3300);
            check("cancel_stops_child_helpers_too", !QFile::exists(base + "/youtube-child-survived.txt"));
        }
        {
            QFile::remove(base + "/youtube-child-survived.txt");
            auto job = std::make_unique<YouTubeDownload>(); bool childStarted = false;
            QObject::connect(job.get(), &YouTubeDownload::progress, &app, [&](QString message) { if (message == "child-started") childStarted = true; });
            job->start(request(9));
            check("shutdown_fixture_child_started", waitFor([&] { return childStarted; }));
            job.reset(); spin(3300);
            check("destruction_stops_child_helpers", !QFile::exists(base + "/youtube-child-survived.txt"));
        }
        {
            YouTubeDownload job; int failed = 0;
            YouTubeDownloadTestAccess::timeout(job, 100);
            QObject::connect(&job, &YouTubeDownload::failed, &app, [&](QString message) { if (message.contains("15 minutes")) ++failed; });
            check("watchdog_stops_stalled_download", job.start(request(9)) && waitFor([&] { return !job.isBusy(); }) && failed == 1);
        }
        {
            auto db = QSqlDatabase::addDatabase("QSQLITE"); db.setDatabaseName(":memory:"); check("library_fixture_open", db.open());
            QSqlQuery sql;
            check("library_fixture_schema", sql.exec("CREATE TABLE dbSongs (songid INTEGER PRIMARY KEY AUTOINCREMENT, discid TEXT, artist TEXT, title TEXT, path TEXT UNIQUE, duration INTEGER, filename TEXT, searchstring TEXT, plays INTEGER DEFAULT 0, lastplay TEXT)"));
            TableModelKaraokeSongs model;
            auto id = model.addDownloadedVideo(completed, "O'Neil é", completedTitle, "00000000001", completedDuration);
            check("completed_video_added_as_local_song", id > 0 && model.getIdForPath(completed) == id && model.getSong(id).artist == "O'Neil é");
            sql.exec("UPDATE dbSongs SET plays=7");
            check("watched_folder_import_is_deduplicated", id > 0 && model.addDownloadedVideo(completed, "Edited artist", "Edited title", "00000000001", completedDuration) == id);
            sql.exec("SELECT count(*),artist,title,plays,path,duration FROM dbSongs"); sql.next();
            check("import_keeps_play_history_and_chosen_metadata", sql.value(0).toInt() == 1 && sql.value(1).toString() == "Edited artist" && sql.value(2).toString() == "Edited title" && sql.value(3).toInt() == 7 && sql.value(4).toString() == completed && sql.value(5).toInt() > 0);
            check("invalid_import_rejected", model.addDownloadedVideo(completed, "", "title", "00000000001", 1000) == -1);
        }
        {
            YouTubeDownloadDialog dialog;
            auto *add = dialog.findChild<QPushButton *>("addDownloadButton");
            check("dialog_requires_completed_download_before_import", add && !add->isEnabled());
            dialog.findChild<QLineEdit *>("youtubeUrl")->setText(request(1).url);
            dialog.findChild<QLineEdit *>("downloaderPath")->setText(program);
            dialog.findChild<QLineEdit *>("ffmpegPath")->setText(program);
            dialog.findChild<QLineEdit *>("denoPath")->setText(program);
            dialog.findChild<QLineEdit *>("downloadFolder")->setText(output.path());
            int imports = 0;
            QObject::connect(&dialog, &YouTubeDownloadDialog::addToLibrary, &app, [&](QString, QString artist, QString title, QString id, int duration) {
                if (artist == "Chosen artist" && !title.isEmpty() && id == "00000000001" && duration > 0) ++imports;
                dialog.libraryImportFinished(true, "Added");
            });
            dialog.findChild<QPushButton *>("downloadButton")->click();
            check("dialog_fills_title_only_after_verification", waitFor([&] { return !dialog.findChild<QLineEdit *>("downloadTitle")->text().isEmpty(); }));
            check("completed_download_does_not_auto_import", imports == 0 && !add->isEnabled());
            dialog.findChild<QLineEdit *>("downloadArtist")->setText("Chosen artist");
            dialog.adjustSize();
            check("download_dialog_preview_saved", dialog.grab().save("youtube-download-dialog.png"));
            add->click();
            check("host_explicitly_approves_library_import", imports == 1 && !add->isEnabled());
            dialog.findChild<QLineEdit *>("youtubeUrl")->setText(request(9).url);
            dialog.show(); dialog.findChild<QPushButton *>("downloadButton")->click(); dialog.close();
            check("closing_dialog_cancels_starting_download", waitFor([&] { return !dialog.isVisible(); }));
        }
        {
            // Real local HTTP server -> same dialog/controller -> real media validator -> library callback.
            YouTubeDownloadDialog dialog;
            for (const auto &name : {"downloaderPath", "ffmpegPath", "denoPath"}) dialog.findChild<QLineEdit *>(name)->setText(program);
            dialog.findChild<QLineEdit *>("downloadFolder")->setText(output.path());
            RemoteControlServer server;
            check("host_http_server_listens", server.listen(QHostAddress::LocalHost, 0));
            int commands=0, imports=0;
            QString importedArtist, importedTitle;
            server.setStatusProvider([&] { return QJsonObject{{"youtube_download", dialog.remoteStatus()}}; });
            server.setLibraryActionHandler([&](const QString &command, const QJsonObject &args) {
                if (command == "guest_submit") return QJsonObject{{"ok", false}, {"error", "Guests cannot download"}};
                ++commands; return dialog.remoteAction(command, args);
            });
            QObject::connect(&dialog, &YouTubeDownloadDialog::addToLibrary, &app,
                [&](QString path, QString artist, QString title, QString id, int duration) {
                    TableModelKaraokeSongs model;
                    const auto song = model.addDownloadedVideo(path, artist, title, id, duration);
                    if (song > 0) { ++imports; importedArtist=artist; importedTitle=title; }
                    dialog.libraryImportFinished(song > 0, song > 0 ? "Added" : "Import failed");
                });
            QNetworkAccessManager network;
            struct Response { int status; QJsonObject json; QByteArray bytes; };
            auto http = [&](QString path, QJsonObject body=QJsonObject{}, bool post=false) {
                QNetworkRequest req(QUrl(QString("http://127.0.0.1:%1").arg(server.serverPort())+path));
                req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
                auto *reply = post ? network.post(req, QJsonDocument(body).toJson(QJsonDocument::Compact)) : network.get(req);
                if (!waitFor([&] { return reply->isFinished(); }, 4000)) reply->abort();
                const auto bytes = reply->readAll();
                Response result{reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), QJsonDocument::fromJson(bytes).object(), bytes};
                delete reply; return result;
            };
            auto call = [&](QString command, QJsonObject args=QJsonObject{}) {
                args.insert("cmd", command); args.insert("pin", server.accessPin());
                return http("/remote", args, true);
            };
            for (const auto &command : {"youtube_download", "youtube_cancel", "youtube_import"}) {
                const auto response = http("/remote", {{"cmd", command}, {"url", request(1).url}}, true);
                check(QString("host_download_requires_pin_")+command, response.status==403 && commands==0);
                const auto get = http("/remote?pin="+server.accessPin()+"&cmd="+command);
                check(QString("host_download_requires_post_")+command, get.status==405 && commands==0);
            }
            check("host_download_wrong_pin_denied", http("/remote", {{"cmd", "youtube_download"}, {"pin", "wrong"}}, true).status==403 && commands==0);
            check("guest_page_has_no_download_controls", !http("/request").bytes.contains("youtubeStart"));
            check("host_page_contains_download_controls", http("/").bytes.contains("youtubeStart"));
            check("guest_command_cannot_reach_downloader", http("/public/request", {{"cmd", "youtube_download"}, {"url", request(1).url}}, true).status==400 && commands==0 && !dialog.remoteStatus().value("busy").toBool());
            check("host_rejects_non_youtube_link", call("youtube_download", {{"url", "file:///secret"}}).status==400);
            check("host_cannot_override_helper_paths", call("youtube_download", {{"url", request(1).url}, {"downloader", "not-installed"}, {"folder", "not-a-folder"}}).status==200);
            auto session=dialog.remoteStatus().value("session").toString();
            check("host_and_desktop_share_busy_download", call("youtube_download", {{"url", request(1).url}}).status==400 && dialog.remoteStatus().value("session").toString()==session);
            check("host_download_completes_without_auto_import", waitFor([&] { return dialog.remoteStatus().value("ready").toBool(); }) && imports==0);
            auto state=http("/status?pin="+server.accessPin()).json.value("youtube_download").toObject();
            check("host_progress_contains_no_local_file_paths", state.value("ready").toBool() && !state.contains("path") && !state.contains("downloader") && !state.value("message").toString().contains(output.path()));
            check("host_stale_import_rejected", call("youtube_import", {{"session", "old"}, {"artist", "Artist"}, {"title", "Title"}}).status==400 && imports==0);
            check("host_empty_artist_rejected", call("youtube_import", {{"session", session}, {"artist", ""}, {"title", "Title"}}).status==400 && imports==0);
            check("host_oversized_title_rejected", call("youtube_import", {{"session", session}, {"artist", "Artist"}, {"title", QString(513, 'x')}}).status==400 && imports==0);
            check("host_control_characters_rejected", call("youtube_import", {{"session", session}, {"artist", "Artist\n"}, {"title", "Ti\ntle"}}).status==400 && imports==0);
            check("host_explicit_import_updates_real_library", call("youtube_import", {{"session", session}, {"artist", "Remote O'Neil"}, {"title", "Remote <title>"}}).status==200 && imports==1 && importedArtist=="Remote O'Neil" && importedTitle=="Remote <title>");
            check("host_repeat_import_cannot_duplicate", call("youtube_import", {{"session", session}, {"artist", "Artist"}, {"title", "Title"}}).status==400 && imports==1);
            check("host_can_start_next_download", call("youtube_download", {{"url", request(9).url}}).status==200);
            auto nextSession=dialog.remoteStatus().value("session").toString();
            check("host_stale_cancel_cannot_stop_new_download", nextSession!=session && call("youtube_cancel", {{"session", session}}).status==400 && dialog.remoteStatus().value("busy").toBool());
            check("host_can_cancel_active_download", call("youtube_cancel", {{"session", nextSession}}).status==200 && waitFor([&] { return !dialog.remoteStatus().value("busy").toBool(); }) && !dialog.remoteStatus().value("ready").toBool());
            check("host_and_desktop_show_same_cancellation", dialog.findChild<QPlainTextEdit *>("downloadStatus")->toPlainText()==dialog.remoteStatus().value("message").toString());
        }
    } catch (const std::exception &error) { check(QString("exception: ") + error.what(), false); }
    writeFile("youtube-download-evidence.json", QJsonDocument(QJsonObject{{"passed", all}, {"live_youtube_tested", false}, {"checks", checks}}).toJson());
    return all ? 0 : 1;
}
