// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef OPENKJ_DLGYOUTUBEDOWNLOAD_H
#define OPENKJ_DLGYOUTUBEDOWNLOAD_H
#include <QDialog>
#include <QJsonObject>
#include "youtubedownload.h"
#include "settings.h"
class QLineEdit;
class QPushButton;
class QPlainTextEdit;
class QProgressBar;

class YouTubeDownloadDialog : public QDialog {
    Q_OBJECT
public:
    explicit YouTubeDownloadDialog(QWidget *parent = nullptr);
    void libraryImportFinished(bool success, const QString &message);
    QJsonObject remoteStatus() const;
    QJsonObject remoteAction(const QString &command, const QJsonObject &args);
signals:
    void addToLibrary(QString path, QString artist, QString title, QString videoId, int durationMs);
protected:
    void reject() override;
    void closeEvent(QCloseEvent *event) override;
private:
    Settings m_settings;
    YouTubeDownload m_download;
    QLineEdit *m_url, *m_downloader, *m_ffmpeg, *m_deno, *m_folder, *m_artist, *m_title;
    QPushButton *m_start, *m_cancel, *m_add, *m_openFolder;
    QPlainTextEdit *m_status;
    QProgressBar *m_progress;
    QString m_path, m_videoId;
    int m_durationMs{0};
    bool m_closeWhenStopped{false}, m_importSucceeded{false};
    QString m_session, m_remoteMessage;
    void startDownload();
    void updateControls();
    void stopped();
};
#endif
