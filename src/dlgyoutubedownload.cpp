// SPDX-License-Identifier: GPL-3.0-or-later
#include "dlgyoutubedownload.h"
#include <QLineEdit>
#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QCloseEvent>

YouTubeDownloadDialog::YouTubeDownloadDialog(QWidget *parent) : QDialog(parent), m_download(this) {
    setWindowTitle(tr("Download a YouTube karaoke video"));
    setWindowIcon(parent ? parent->windowIcon() : QIcon());
    setMinimumWidth(620);
    auto *layout = new QVBoxLayout(this);
    auto *intro = new QLabel(tr("Paste a link to a karaoke video. The completed video will be checked before you add it to your library. Downloading does not change the current singer or queue."), this);
    intro->setWordWrap(true); layout->addWidget(intro);
    auto *form = new QFormLayout;
    m_url = new QLineEdit(this); m_url->setMaxLength(2048); m_url->setObjectName("youtubeUrl"); m_url->setPlaceholderText("https://www.youtube.com/watch?v=…");
    form->addRow(tr("YouTube link"), m_url);
    auto options = m_settings.youtubeDownloadOptions();
    auto browseRow = [this](QLineEdit *edit, bool directory) {
        auto *row = new QWidget(this); auto *box = new QHBoxLayout(row); box->setContentsMargins(0,0,0,0);
        auto *button = new QPushButton(tr("Browse…"), row); button->setObjectName("downloadBrowse"); box->addWidget(edit); box->addWidget(button);
        connect(button, &QPushButton::clicked, this, [this, edit, directory] {
            QString path = directory ? QFileDialog::getExistingDirectory(this, tr("Choose a folder for completed videos"), edit->text())
                : QFileDialog::getOpenFileName(this, tr("Select the installed helper program"), edit->text(),
#ifdef Q_OS_WIN
                    tr("Programs (*.exe)")
#else
                    tr("All files (*)")
#endif
                );
            if (!path.isEmpty()) edit->setText(QDir::fromNativeSeparators(path));
        });
        return row;
    };
    m_folder = new QLineEdit(options.value("folder", QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString(), this);
    m_folder->setObjectName("downloadFolder"); form->addRow(tr("Save videos in"), browseRow(m_folder, true)); layout->addLayout(form);
    auto *setup = new QGroupBox(tr("One-time setup"), this); auto *setupForm = new QFormLayout(setup);
    auto tool = [this, &options](const QString &key, const QString &program) {
        const auto adjacent = QDir(QFileInfo(options.value("downloader").toString()).absolutePath()).filePath(program
#ifdef Q_OS_WIN
            + ".exe"
#endif
        );
        auto found = QStandardPaths::findExecutable(program);
        if (found.isEmpty() && QFileInfo(adjacent).isFile()) found = adjacent;
        return new QLineEdit(options.value(key, found).toString(), this);
    };
    m_downloader = tool("downloader", "yt-dlp"); m_downloader->setObjectName("downloaderPath");
    m_ffmpeg = tool("ffmpeg", "ffmpeg"); m_ffmpeg->setObjectName("ffmpegPath");
    m_deno = tool("deno", "deno"); m_deno->setObjectName("denoPath");
    setupForm->addRow(tr("Downloader (yt-dlp)"), browseRow(m_downloader, false));
    setupForm->addRow(tr("Video merger (ffmpeg)"), browseRow(m_ffmpeg, false));
    setupForm->addRow(tr("YouTube helper (Deno)"), browseRow(m_deno, false));
    auto *help = new QPushButton(tr("Helper setup instructions"), setup);
    connect(help, &QPushButton::clicked, this, [] {
        QDesktopServices::openUrl(QUrl("https://github.com/speedhawk77-art/OpenKJ/blob/feature/youtube-download-first/community/YOUTUBE_DOWNLOAD_FIRST_v9.md"));
    });
    setupForm->addRow(tr("Install these separately once."), help); layout->addWidget(setup);
    m_progress = new QProgressBar(this); m_progress->setRange(0,1); m_progress->setValue(0); layout->addWidget(m_progress);
    m_status = new QPlainTextEdit(this); m_status->setObjectName("downloadStatus");
    m_status->setReadOnly(true); m_status->setFixedHeight(90); m_status->setMaximumBlockCount(30);
    m_status->setAccessibleName(tr("Download status"));
    m_status->setPlainText(tr("Ready. Downloads need an internet connection.")); layout->addWidget(m_status);
    auto *buttons = new QHBoxLayout;
    m_start = new QPushButton(tr("Download video"), this); m_start->setObjectName("downloadButton");
    m_cancel = new QPushButton(tr("Cancel download"), this); m_cancel->setObjectName("cancelDownloadButton");
    buttons->addWidget(m_start); buttons->addWidget(m_cancel); buttons->addStretch(); layout->addLayout(buttons);
    auto *details = new QGroupBox(tr("Add the completed video to your library"), this); auto *detailsForm = new QFormLayout(details);
    m_artist = new QLineEdit(this); m_artist->setObjectName("downloadArtist"); m_artist->setPlaceholderText(tr("Artist or karaoke channel"));
    m_title = new QLineEdit(this); m_title->setObjectName("downloadTitle");
    m_artist->setMaxLength(256); m_title->setMaxLength(512);
    detailsForm->addRow(tr("Artist"), m_artist); detailsForm->addRow(tr("Title"), m_title);
    auto *importButtons = new QHBoxLayout;
    m_add = new QPushButton(tr("Add to library"), this); m_add->setObjectName("addDownloadButton");
    m_openFolder = new QPushButton(tr("Open folder"), this);
    importButtons->addWidget(m_add); importButtons->addWidget(m_openFolder); importButtons->addStretch(); detailsForm->addRow(importButtons); layout->addWidget(details);
    auto *close = new QPushButton(tr("Close"), this); layout->addWidget(close, 0, Qt::AlignRight);
    for (auto *button : findChildren<QPushButton *>()) button->setAutoDefault(false);
    connect(close, &QPushButton::clicked, this, &YouTubeDownloadDialog::reject);
    connect(m_start, &QPushButton::clicked, this, &YouTubeDownloadDialog::startDownload);
    connect(m_cancel, &QPushButton::clicked, &m_download, &YouTubeDownload::cancel);
    connect(&m_download, &YouTubeDownload::progress, m_status, &QPlainTextEdit::setPlainText);
    connect(&m_download, &YouTubeDownload::failed, this, [this](const QString &error) { m_status->setPlainText(error); stopped(); });
    connect(&m_download, &YouTubeDownload::cancelled, this, [this] { m_status->setPlainText(tr("Download cancelled. Nothing was added to the library.")); stopped(); });
    connect(&m_download, &YouTubeDownload::ready, this, [this](const QString &path, const QString &title, const QString &id, int duration) {
        m_path = path; m_videoId = id; m_durationMs = duration; m_title->setText(title);
        m_status->setPlainText(tr("Video saved and checked. Confirm its artist and title, then click Add to library.\n%1").arg(path)); stopped();
    });
    connect(m_title, &QLineEdit::textChanged, this, &YouTubeDownloadDialog::updateControls);
    connect(m_artist, &QLineEdit::textChanged, this, &YouTubeDownloadDialog::updateControls);
    connect(m_add, &QPushButton::clicked, this, [this] {
        m_add->setEnabled(false);
        emit addToLibrary(m_path, m_artist->text().trimmed(), m_title->text().trimmed(), m_videoId, m_durationMs);
    });
    connect(m_openFolder, &QPushButton::clicked, this, [this] { QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(m_path).absolutePath())); });
    updateControls();
}

void YouTubeDownloadDialog::startDownload() {
    m_path.clear(); m_videoId.clear(); m_durationMs = 0; m_title->clear(); m_artist->clear();
    YouTubeDownload::Request request{m_url->text(), m_downloader->text().trimmed(), m_ffmpeg->text().trimmed(), m_deno->text().trimmed(), m_folder->text().trimmed()};
    if (m_download.start(request)) {
        m_settings.setYoutubeDownloadOptions({{"downloader", request.downloader}, {"ffmpeg", request.ffmpeg}, {"deno", request.deno}, {"folder", request.destination}});
    }
    updateControls();
}

void YouTubeDownloadDialog::updateControls() {
    const auto busy = m_download.isBusy();
    for (auto *field : {m_url, m_folder, m_downloader, m_ffmpeg, m_deno}) field->setEnabled(!busy);
    for (auto *button : findChildren<QPushButton *>("downloadBrowse")) button->setEnabled(!busy);
    m_start->setEnabled(!busy); m_cancel->setEnabled(busy);
    m_artist->setEnabled(!busy && !m_path.isEmpty()); m_title->setEnabled(!busy && !m_path.isEmpty());
    m_add->setEnabled(!busy && !m_path.isEmpty() && !m_artist->text().trimmed().isEmpty() && !m_title->text().trimmed().isEmpty());
    m_openFolder->setEnabled(!m_path.isEmpty());
    m_progress->setRange(0, busy ? 0 : 1); m_progress->setValue(m_path.isEmpty() ? 0 : 1);
}

void YouTubeDownloadDialog::stopped() {
    updateControls();
    if (m_closeWhenStopped) { m_closeWhenStopped = false; QDialog::reject(); }
}

void YouTubeDownloadDialog::libraryImportFinished(bool success, const QString &message) {
    m_status->setPlainText(message);
    if (success) { m_path.clear(); m_artist->clear(); m_title->clear(); }
    updateControls();
}

void YouTubeDownloadDialog::reject() {
    if (m_download.isBusy()) { m_closeWhenStopped = true; m_download.cancel(); }
    else QDialog::reject();
}

void YouTubeDownloadDialog::closeEvent(QCloseEvent *event) {
    if (m_download.isBusy()) { event->ignore(); reject(); }
    else QDialog::closeEvent(event);
}
