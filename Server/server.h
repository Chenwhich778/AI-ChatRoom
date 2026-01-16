#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QSet>
#include <QJsonObject>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    void Connect(int port);
private:
    struct ClientInfo {
        QString account;
        QString name;
        QSet<QString> rooms;  // 用户可以加入多个房间
        bool loggedIn = false;
    };

    QHash<QTcpSocket*, ClientInfo> clients;
    QHash<QTcpSocket*, QByteArray> buffers;
    QHash<QString, QSet<QTcpSocket*>> rooms;

    void handleMessage(QTcpSocket *client, const QJsonObject &obj);
    void sendJson(QTcpSocket *client, const QJsonObject &obj);
    void sendRoomList(QTcpSocket *client);
    void broadcastRoomList();
    void broadcastToRoom(const QString &room, const QJsonObject &obj);
    void removeFromRoom(QTcpSocket *client, const QString &room);
    void removeFromAllRooms(QTcpSocket *client);

protected:
    void incomingConnection(qintptr handle) override;

private slots:
    void onReadyRead(QTcpSocket *client);
    void onDisconnected(QTcpSocket *client);
signals:
    void logMessage(const QString &message);
};

#endif // SERVER_H
