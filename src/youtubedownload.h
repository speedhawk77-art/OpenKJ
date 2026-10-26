// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef OPENKJ_YOUTUBEDOWNLOAD_H
#define OPENKJ_YOUTUBEDOWNLOAD_H

#include <QObject>
#include <QFutureWatcher>
#include <QTimer>
#include <QTemporaryDir>
#include <atomic>
#include <memory>

class DownloadProcess;

class YouTubeDownload : public QObject {
    Q_OBJECT
public:
    struct Request {
        QString url, downloader, ffmpeg, deno, destination;
    };
    explicit YouTubeDownload(QObject *parent = nullptr);
    ~YouTubeDownload() override;
    bool start(const Request &request);
    bool isBusy() const { return m_busy; }
    static QString canonicalUrl(const QString &text);

public slots:
    void cancel();

signals:
    void progress(QString message);
    void ready(QString path, QString title, QString videoId, int durationMs);
    void failed(QString message);
    void cancelled();

private:
    friend struct YouTubeDownloadTestAccess;
    struct Result {
        QString path, title, videoId, error;
        int durationMs{0};
        bool cancelled{false};
    };
    std::unique_ptr<DownloadProcess> m_process;
    std::shared_ptr<QTemporaryDir> m_temporary;
    std::shared_ptr<std::atomic_bool> m_cancelled;
    QFutureWatcher<Result> m_watcher;
    QTimer m_deadline;
    Request m_request;
    QByteArray m_errorTail, m_outputTail;
    bool m_busy{false}, m_timedOut{false};
    void processFinished(int code, int status);
    void fail(const QString &message);
    void reset();
    static Result verifyAndSave(const QString &source, const QString &destination,
                                const QString &title, const QString &videoId,
                                const std::shared_ptr<std::atomic_bool> &cancelled);
};
#endif
