#include "logindialog.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , socket(new QTcpSocket(this))
{
    setupUI();
    connect(socket, &QTcpSocket::readyRead, this, &LoginDialog::onSocketReadyRead);
}

void LoginDialog::setupUI()
{
    setWindowTitle("AI-ChatRoom - 登录");
    setFixedSize(360, 320);
    setStyleSheet(R"(
        QDialog {
            background-color: #2b2d30;
        }
        QLabel {
            color: #d0d0d0;
            font-family: "Microsoft YaHei";
            font-size: 10pt;
        }
        QLineEdit {
            border: 1px solid #5a5a5a;
            border-radius: 6px;
            background-color: #3c3f41;
            color: #d0d0d0;
            padding: 6px;
            font-family: "Microsoft YaHei";
            font-size: 10pt;
        }
        QLineEdit:focus {
            border: 1px solid #5ac2c6;
        }
        QPushButton {
            border: none;
            border-radius: 6px;
            background-color: #5ac2c6;
            color: white;
            padding: 8px 16px;
            font-family: "Microsoft YaHei";
            font-size: 10pt;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1fab89;
        }
        QPushButton:disabled {
            background-color: #555555;
            color: #888888;
        }
        QGroupBox {
            border: 1px solid #5a5a5a;
            border-radius: 6px;
            margin-top: 10px;
            color: #d0d0d0;
            font-family: "Microsoft YaHei";
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 服务器连接区域
    auto *serverGroup = new QGroupBox("服务器连接", this);
    auto *serverLayout = new QHBoxLayout(serverGroup);
    ipEdit = new QLineEdit(this);
    ipEdit->setPlaceholderText("服务器IP");
    ipEdit->setText("127.0.0.1");
    portEdit = new QLineEdit(this);
    portEdit->setPlaceholderText("端口");
    portEdit->setText("12345");
    portEdit->setMaximumWidth(80);
    connectButton = new QPushButton("连接", this);
    serverLayout->addWidget(ipEdit);
    serverLayout->addWidget(portEdit);
    serverLayout->addWidget(connectButton);
    mainLayout->addWidget(serverGroup);

    // 登录区域
    auto *loginGroup = new QGroupBox("账号登录", this);
    auto *formLayout = new QFormLayout(loginGroup);
    accountEdit = new QLineEdit(this);
    accountEdit->setPlaceholderText("请输入账号");
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("请输入密码");
    passwordEdit->setEchoMode(QLineEdit::Password);
    nicknameEdit = new QLineEdit(this);
    nicknameEdit->setPlaceholderText("可选，默认使用账号");
    formLayout->addRow("账号:", accountEdit);
    formLayout->addRow("密码:", passwordEdit);
    formLayout->addRow("昵称:", nicknameEdit);
    mainLayout->addWidget(loginGroup);

    loginButton = new QPushButton("登录", this);
    loginButton->setEnabled(false);
    mainLayout->addWidget(loginButton);

    statusLabel = new QLabel("请先连接服务器", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    connect(connectButton, &QPushButton::clicked, this, &LoginDialog::onConnectClicked);
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(accountEdit, &QLineEdit::returnPressed, this, [this]() { passwordEdit->setFocus(); });
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onConnectClicked()
{
    socket->connectToHost(ipEdit->text(), portEdit->text().toInt());
    if (socket->waitForConnected(2000)) {
        connected = true;
        statusLabel->setText("已连接服务器，请登录");
        loginButton->setEnabled(true);
        accountEdit->setFocus();
        connectButton->setEnabled(false);
    } else {
        statusLabel->setText("连接失败，请检查服务器");
    }
}

void LoginDialog::onLoginClicked()
{
    if (!connected) {
        statusLabel->setText("请先连接服务器");
        return;
    }
    QString account = accountEdit->text().trimmed();
    QString password = passwordEdit->text();
    if (account.isEmpty() || password.isEmpty()) {
        statusLabel->setText("账号和密码不能为空");
        return;
    }

    QJsonObject obj;
    obj["type"] = "login";
    obj["account"] = account;
    obj["password"] = password;
    obj["name"] = nicknameEdit->text().trimmed().isEmpty() ? account : nicknameEdit->text().trimmed();
    sendJson(obj);
    statusLabel->setText("登录中...");
}

void LoginDialog::onSocketReadyRead()
{
    buffer.append(socket->readAll());
    while (true) {
        int idx = buffer.indexOf('\n');
        if (idx < 0) break;
        QByteArray line = buffer.left(idx);
        buffer.remove(0, idx + 1);
        if (line.trimmed().isEmpty()) continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(line, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            handleMessage(doc.object());
        }
    }
}

void LoginDialog::handleMessage(const QJsonObject &obj)
{
    QString type = obj.value("type").toString();
    if (type == "login_ok") {
        loginSuccess = true;
        QString nickname = obj.value("name").toString();
        emit loginSuccessful(accountEdit->text().trimmed(), nickname);
        // 不立即关闭，等待接收room_list
        statusLabel->setText("正在加载聊天室列表...");
    } else if (type == "login_fail") {
        statusLabel->setText(obj.value("message").toString());
    } else if (type == "room_list" && loginSuccess) {
        // 接收到房间列表后才关闭对话框
        const QJsonArray arr = obj.value("rooms").toArray();
        roomList.clear();
        for (const QJsonValue &val : arr) {
            roomList.append(val.toString());
        }
        accept();
    }
}

void LoginDialog::sendJson(const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    socket->write(data);
}

QString LoginDialog::getAccount() const { return accountEdit->text().trimmed(); }
QString LoginDialog::getPassword() const { return passwordEdit->text(); }
QString LoginDialog::getNickname() const { 
    return nicknameEdit->text().trimmed().isEmpty() ? accountEdit->text().trimmed() : nicknameEdit->text().trimmed(); 
}
QString LoginDialog::getServerIp() const { return ipEdit->text(); }
int LoginDialog::getServerPort() const { return portEdit->text().toInt(); }
QTcpSocket* LoginDialog::getSocket() { return socket; }
QStringList LoginDialog::getRoomList() const { return roomList; }
