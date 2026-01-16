#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTcpSocket>
#include <QJsonObject>

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    
    QString getAccount() const;
    QString getPassword() const;
    QString getNickname() const;
    QString getServerIp() const;
    int getServerPort() const;
    QTcpSocket* getSocket();
    QStringList getRoomList() const;

signals:
    void loginSuccessful(const QString &account, const QString &nickname);

private slots:
    void onConnectClicked();
    void onLoginClicked();
    void onSocketReadyRead();

private:
    void sendJson(const QJsonObject &obj);
    void handleMessage(const QJsonObject &obj);
    void setupUI();

    QLineEdit *ipEdit;
    QLineEdit *portEdit;
    QPushButton *connectButton;
    QLineEdit *accountEdit;
    QLineEdit *passwordEdit;
    QLineEdit *nicknameEdit;
    QPushButton *loginButton;
    QLabel *statusLabel;

    QTcpSocket *socket;
    QByteArray buffer;
    QStringList roomList;
    bool connected = false;
    bool loginSuccess = false;
};

#endif // LOGINDIALOG_H
