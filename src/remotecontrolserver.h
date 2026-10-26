#pragma once

#include <QTcpServer>
#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QStringList>
#include <functional>

class QTcpSocket;

// Embedded offline-only HTTP remote. Socket activity and playback run on the Qt GUI event loop.
// Only access from a trusted local hotspot; the control and status APIs require a session PIN.
class RemoteControlServer final : public QTcpServer {
    Q_OBJECT
public:
    explicit RemoteControlServer(QObject *parent = nullptr);
    bool start(quint16 preferredPort = 8081);
    QString accessPin() const { return m_pin; }
    QStringList localUrls() const;
    void setStatusProvider(std::function<QJsonObject()> callback);
    void setSongSearchProvider(std::function<QJsonObject(const QString &)> callback);
    void setLibraryActionHandler(std::function<QJsonObject(const QString &, const QJsonObject &)> callback);

    void setHostProviders(std::function<QJsonObject(const QJsonObject &)> state,
                          std::function<QJsonObject(const QJsonObject &)> search,
                          std::function<QJsonObject(const QString &, const QJsonObject &)> action);

signals:
    void playRequested();
    void pauseRequested();
    void playPauseRequested();
    void nextRequested();
    void stopRequested();
    void volumeUpRequested();
    void volumeDownRequested();

    void breakPlayRequested();
    void breakPauseRequested();
    void breakToggleRequested();
    void breakNextRequested();
    void breakPreviousRequested();
    void breakStopRequested();
    void breakVolumeUpRequested();
    void breakVolumeDownRequested();
    void breakFadeOutRequested();
    void breakFadeInRequested();

private:
    void acceptConnections();
    void readFrom(QTcpSocket *socket);
    void processRequest(QTcpSocket *socket, const QByteArray &header, const QByteArray &body);
    void reply(QTcpSocket *socket, int status, const char *description,
               const QByteArray &contentType, const QByteArray &body);

    QHash<QTcpSocket *, QByteArray> m_pending;
    QString m_pin;
    QHash<QString,QPair<int,qint64>> m_failedPins;
    std::function<QJsonObject(const QJsonObject &)> m_hostState, m_hostSearch;
    std::function<QJsonObject(const QString &, const QJsonObject &)> m_hostAction;
    std::function<QJsonObject()> m_statusProvider;
    std::function<QJsonObject(const QString &)> m_songSearchProvider;
    std::function<QJsonObject(const QString &, const QJsonObject &)> m_libraryActionHandler;
};
