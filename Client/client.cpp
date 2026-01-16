//made by kevinwu06
#include "client.h"
#include "ui_client.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);
    this->setWindowTitle("DuckChat客户端");
    ui->receive->setText(text);

    connect(ui->ip, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->port, SIGNAL(returnPressed()), this, SLOT(on_connectButton_clicked()));
    connect(ui->accountEdit, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->passwordEdit, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->nameEdit, SIGNAL(returnPressed()), this, SLOT(next()));
    connect(ui->roomNameEdit, SIGNAL(returnPressed()), this, SLOT(on_createRoomButton_clicked()));
    connect(ui->send, SIGNAL(returnPressed()), this, SLOT(on_sendButton_clicked()));
    connect(ui->loginButton, SIGNAL(clicked()), this, SLOT(on_loginButton_clicked()));
    connect(ui->joinButton, SIGNAL(clicked()), this, SLOT(on_joinButton_clicked()));
    connect(ui->createRoomButton, SIGNAL(clicked()), this, SLOT(on_createRoomButton_clicked()));
    connect(&socket, SIGNAL(readyRead()), this, SLOT(receiveData()));

    ui->loginButton->setEnabled(false);
    ui->joinButton->setEnabled(false);
    ui->createRoomButton->setEnabled(false);
    setChatEnabled(false);
}

void Client::receiveData()
{
    buffer.append(socket.readAll());
    while (true) {
        int newlineIndex = buffer.indexOf('\n');
        if (newlineIndex < 0) {
            break;
        }
        QByteArray line = buffer.left(newlineIndex);
        buffer.remove(0, newlineIndex + 1);

        if (line.trimmed().isEmpty()) {
            continue;
        }

        QJsonParseError error{};
        QJsonDocument doc = QJsonDocument::fromJson(line, &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            appendSystemMessage("收到无法解析的消息");
            continue;
        }
        handleMessage(doc.object());
    }
}

void Client::on_connectButton_clicked()
{
    socket.connectToHost(ui->ip->text(), ui->port->text().toInt());
    if (socket.waitForConnected(1000)) {
        connected = true;
        appendSystemMessage("成功连接服务器...");
        ui->loginButton->setEnabled(true);
        ui->accountEdit->setFocus();
    } else {
        appendSystemMessage("连接失败...");
    }
}

void Client::on_loginButton_clicked()
{
    if (!connected) {
        appendSystemMessage("请先连接服务器");
        return;
    }
    const QString account = ui->accountEdit->text().trimmed();
    const QString password = ui->passwordEdit->text();
    const QString name = ui->nameEdit->text().trimmed();

    if (account.isEmpty() || password.isEmpty()) {
        appendSystemMessage("账号或密码不能为空");
        return;
    }

    QJsonObject obj;
    obj["type"] = "login";
    obj["account"] = account;
    obj["password"] = password;
    obj["name"] = name;
    sendJson(obj);
}

void Client::on_joinButton_clicked()
{
    if (!loggedIn) {
        appendSystemMessage("请先登录");
        return;
    }
    const QString room = ui->roomCombo->currentText().trimmed();
    if (room.isEmpty() || room == "请选择聊天室") {
        appendSystemMessage("请选择聊天室");
        return;
    }

    QJsonObject obj;
    obj["type"] = "join_room";
    obj["room"] = room;
    sendJson(obj);
}

void Client::on_createRoomButton_clicked()
{
    if (!loggedIn) {
        appendSystemMessage("请先登录");
        return;
    }
    const QString room = ui->roomNameEdit->text().trimmed();
    if (room.isEmpty()) {
        appendSystemMessage("请输入聊天室名称");
        return;
    }

    QJsonObject obj;
    obj["type"] = "create_room";
    obj["room"] = room;
    sendJson(obj);
}

void Client::on_sendButton_clicked()
{
    if (currentRoom.isEmpty()) {
        appendSystemMessage("请先加入聊天室");
        return;
    }
    QString message = ui->send->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    QJsonObject obj;
    obj["type"] = "chat";
    obj["message"] = message;
    sendJson(obj);
    ui->send->clear();
    ui->send->setFocus();
}

void Client::next()
{
    QLineEdit *lineEdit = (QLineEdit *)sender();
    if (lineEdit == ui->ip) {
        ui->port->setFocus();
    } else if (lineEdit == ui->accountEdit) {
        ui->passwordEdit->setFocus();
    } else if (lineEdit == ui->passwordEdit) {
        ui->nameEdit->setFocus();
    } else if (lineEdit == ui->nameEdit) {
        ui->roomNameEdit->setFocus();
    }
}

void Client::sendJson(const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    QByteArray line = doc.toJson(QJsonDocument::Compact);
    line.append('\n');
    socket.write(line);
}

void Client::appendSystemMessage(const QString &message)
{
    text.append(message).append("\n");
    ui->receive->setText(text);
}

void Client::handleMessage(const QJsonObject &obj)
{
    const QString type = obj.value("type").toString();
    if (type == "login_ok") {
        loggedIn = true;
        appendSystemMessage(obj.value("message").toString());
        ui->joinButton->setEnabled(true);
        ui->createRoomButton->setEnabled(true);
        return;
    }
    if (type == "login_fail") {
        loggedIn = false;
        appendSystemMessage(obj.value("message").toString());
        return;
    }
    if (type == "room_list") {
        QStringList rooms;
        const QJsonArray arr = obj.value("rooms").toArray();
        for (const QJsonValue &val : arr) {
            rooms.append(val.toString());
        }
        updateRoomList(rooms);
        return;
    }
    if (type == "create_room_ok") {
        appendSystemMessage(obj.value("message").toString());
        ui->roomNameEdit->clear();
        return;
    }
    if (type == "create_room_fail") {
        appendSystemMessage(obj.value("message").toString());
        return;
    }
    if (type == "join_room_ok") {
        currentRoom = obj.value("room").toString();
        appendSystemMessage(obj.value("message").toString() + " -> " + currentRoom);
        setChatEnabled(true);
        ui->send->setFocus();
        return;
    }
    if (type == "join_room_fail") {
        appendSystemMessage(obj.value("message").toString());
        return;
    }
    if (type == "chat") {
        const QString from = obj.value("from").toString();
        const QString message = obj.value("message").toString();
        const QString time = obj.value("time").toString();
        text.append("[" + time + "] " + from + ": " + message + "\n");
        ui->receive->setText(text);
        return;
    }
    if (type == "system") {
        appendSystemMessage(obj.value("message").toString());
        return;
    }
}

void Client::updateRoomList(const QStringList &rooms)
{
    ui->roomCombo->clear();
    ui->roomCombo->addItem("请选择聊天室");
    for (const QString &room : rooms) {
        ui->roomCombo->addItem(room);
    }
}

void Client::setChatEnabled(bool enabled)
{
    ui->send->setEnabled(enabled);
    ui->sendButton->setEnabled(enabled);
}

