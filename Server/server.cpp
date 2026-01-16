#include "server.h"

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QTextStream>

Server::Server(QObject *parent)
    : QTcpServer{parent}
{
}

void Server::Connect(int port)
{
    if (!listen(QHostAddress::Any, port)) {
        QTextStream(stderr) << "Failed to start server: " << errorString() << "\n";
        return;
    }
    QTextStream(stdout) << "Server successfully bound to port " << port << "\n";
}

void Server::incomingConnection(qintptr handle)
{
    auto *client = new QTcpSocket(this);
    if (!client->setSocketDescriptor(handle)) {
        client->deleteLater();
        return;
    }

    clients.insert(client, ClientInfo{});
    buffers.insert(client, QByteArray{});

    connect(client, &QTcpSocket::readyRead, this, [this, client]() {
        onReadyRead(client);
    });
    connect(client, &QTcpSocket::disconnected, this, [this, client]() {
        onDisconnected(client);
    });

    emit logMessage("Client connected");
}

void Server::onReadyRead(QTcpSocket *client)
{
    if (!client || !clients.contains(client)) {
        return;
    }

    QByteArray &buffer = buffers[client];
    buffer.append(client->readAll());

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
            QJsonObject fail;
            fail["type"] = "system";
            fail["message"] = "Invalid message format.";
            sendJson(client, fail);
            continue;
        }

        handleMessage(client, doc.object());
    }
}

void Server::onDisconnected(QTcpSocket *client)
{
    if (!client) {
        return;
    }

    if (clients.contains(client)) {
        const ClientInfo info = clients.value(client);
        // 通知所有已加入的房间
        for (const QString &room : info.rooms) {
            QJsonObject leaveMsg;
            leaveMsg["type"] = "system";
            leaveMsg["room"] = room;
            leaveMsg["message"] = info.name.isEmpty() ? "用户离开聊天室" : info.name + " 离开聊天室";
            broadcastToRoom(room, leaveMsg);
        }
        removeFromAllRooms(client);
        clients.remove(client);
    }

    buffers.remove(client);
    client->deleteLater();
    emit logMessage("Client disconnected");;
}

void Server::handleMessage(QTcpSocket *client, const QJsonObject &obj)
{
    const QString type = obj.value("type").toString();
    if (type == "login") {
        const QString account = obj.value("account").toString().trimmed();
        const QString password = obj.value("password").toString();
        const QString name = obj.value("name").toString().trimmed();

        if (account.isEmpty() || password.isEmpty()) {
            QJsonObject fail;
            fail["type"] = "login_fail";
            fail["message"] = "账号或密码不能为空";
            sendJson(client, fail);
            return;
        }

        ClientInfo &info = clients[client];
        info.account = account;
        info.name = name.isEmpty() ? account : name;
        info.loggedIn = true;

        QJsonObject ok;
        ok["type"] = "login_ok";
        ok["message"] = "登录成功";
        ok["name"] = info.name;
        sendJson(client, ok);

        sendRoomList(client);
        return;
    }

    if (!clients.contains(client) || !clients[client].loggedIn) {
        QJsonObject fail;
        fail["type"] = "system";
        fail["message"] = "请先登录";
        sendJson(client, fail);
        return;
    }

    if (type == "create_room") {
        const QString room = obj.value("room").toString().trimmed();
        if (room.isEmpty()) {
            QJsonObject fail;
            fail["type"] = "create_room_fail";
            fail["message"] = "聊天室名称不能为空";
            sendJson(client, fail);
            return;
        }

        if (!rooms.contains(room)) {
            rooms.insert(room, QSet<QTcpSocket*>{});
            QJsonObject ok;
            ok["type"] = "create_room_ok";
            ok["room"] = room;
            ok["message"] = "聊天室创建成功";
            sendJson(client, ok);
            broadcastRoomList();
        } else {
            QJsonObject fail;
            fail["type"] = "create_room_fail";
            fail["message"] = "聊天室已存在";
            sendJson(client, fail);
        }
        return;
    }

    if (type == "join_room") {
        const QString room = obj.value("room").toString().trimmed();
        if (room.isEmpty() || !rooms.contains(room)) {
            QJsonObject fail;
            fail["type"] = "join_room_fail";
            fail["message"] = "聊天室不存在";
            sendJson(client, fail);
            return;
        }

        // 检查是否已经在这个房间
        if (clients[client].rooms.contains(room)) {
            QJsonObject ok;
            ok["type"] = "join_room_ok";
            ok["room"] = room;
            ok["message"] = "已在聊天室中";
            sendJson(client, ok);
            return;
        }

        // 加入新房间（不离开其他房间）
        rooms[room].insert(client);
        clients[client].rooms.insert(room);

        QJsonObject ok;
        ok["type"] = "join_room_ok";
        ok["room"] = room;
        ok["message"] = "加入聊天室成功";
        sendJson(client, ok);

        QJsonObject sys;
        sys["type"] = "system";
        sys["room"] = room;
        sys["message"] = clients[client].name + " 加入聊天室";
        broadcastToRoom(room, sys);
        return;
    }

    if (type == "chat") {
        const QString room = obj.value("room").toString().trimmed();
        const QString message = obj.value("message").toString();
        ClientInfo &info = clients[client];
        
        // 检查用户是否在指定的房间
        if (room.isEmpty() || !info.rooms.contains(room)) {
            QJsonObject fail;
            fail["type"] = "system";
            fail["message"] = "你不在该聊天室中";
            sendJson(client, fail);
            return;
        }

        QJsonObject chat;
        chat["type"] = "chat";
        chat["room"] = room;
        chat["from"] = info.name;
        chat["message"] = message;
        chat["time"] = QDateTime::currentDateTime().toString("HH:mm:ss");
        broadcastToRoom(room, chat);
        return;
    }

    if (type == "leave_room") {
        const QString room = obj.value("room").toString().trimmed();
        ClientInfo &info = clients[client];
        
        if (room.isEmpty() || !rooms.contains(room)) {
            return;
        }
        
        // Remove client from room
        if (info.rooms.contains(room)) {
            info.rooms.remove(room);
            rooms[room].remove(client);
            
            // Broadcast leave message to remaining members
            QJsonObject sys;
            sys["type"] = "system";
            sys["room"] = room;
            sys["message"] = info.name + " 离开了聊天室";
            broadcastToRoom(room, sys);
            
            // Remove empty room
            if (rooms[room].isEmpty()) {
                rooms.remove(room);
                broadcastRoomList();
            }
        }
        return;
    }
}

void Server::sendJson(QTcpSocket *client, const QJsonObject &obj)
{
    if (!client) {
        return;
    }
    QJsonDocument doc(obj);
    QByteArray line = doc.toJson(QJsonDocument::Compact);
    line.append('\n');
    client->write(line);
}

void Server::sendRoomList(QTcpSocket *client)
{
    QJsonArray roomArray;
    for (auto it = rooms.constBegin(); it != rooms.constEnd(); ++it) {
        roomArray.append(it.key());
    }
    QJsonObject list;
    list["type"] = "room_list";
    list["rooms"] = roomArray;
    sendJson(client, list);
}

void Server::broadcastRoomList()
{
    for (auto it = clients.constBegin(); it != clients.constEnd(); ++it) {
        if (it.value().loggedIn) {
            sendRoomList(it.key());
        }
    }
}

void Server::broadcastToRoom(const QString &room, const QJsonObject &obj)
{
    if (!rooms.contains(room)) {
        return;
    }
    for (QTcpSocket *client : rooms[room]) {
        sendJson(client, obj);
    }
}

void Server::removeFromRoom(QTcpSocket *client, const QString &room)
{
    if (!client || !clients.contains(client)) {
        return;
    }
    if (!room.isEmpty() && rooms.contains(room)) {
        rooms[room].remove(client);
        clients[client].rooms.remove(room);
        if (rooms[room].isEmpty()) {
            rooms.remove(room);
            broadcastRoomList();
        }
    }
}

void Server::removeFromAllRooms(QTcpSocket *client)
{
    if (!client || !clients.contains(client)) {
        return;
    }
    ClientInfo &info = clients[client];
    for (const QString &room : qAsConst(info.rooms)) {
        if (rooms.contains(room)) {
            rooms[room].remove(client);
            if (rooms[room].isEmpty()) {
                rooms.remove(room);
            }
        }
    }
    info.rooms.clear();
    broadcastRoomList();
}

