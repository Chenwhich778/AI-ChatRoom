#ifndef CLIENT_H
#define CLIENT_H

#include <QWidget>
#include <QObject>
#include <QTcpSocket>
#include <QLineEdit>
#include <qbytearray.h>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QWidget
{
    Q_OBJECT

public:
    Client(QWidget *parent = nullptr);
    void Connect();
private slots:
    void receiveData();
    void on_connectButton_clicked();
    void on_loginButton_clicked();
    void on_joinButton_clicked();
    void on_createRoomButton_clicked();
    void on_sendButton_clicked();
    void next();
private:
    void sendJson(const QJsonObject &obj);
    void appendSystemMessage(const QString &message);
    void handleMessage(const QJsonObject &obj);
    void updateRoomList(const QStringList &rooms);
    void setChatEnabled(bool enabled);

    QTcpSocket socket;
    Ui::Client *ui;
    QString text;
    QByteArray buffer;
    QString currentRoom;
    bool loggedIn = false;
    bool connected = false;
};
#endif // CLIENT_H
