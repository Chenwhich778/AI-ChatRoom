#include "mainwindow.h"
#include "roommanager.h"
#include "chatwidget.h"
#include "aiassistant.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QTextStream>
#include <QCoreApplication>
#include <QFileInfo>

MainWindow::MainWindow(QTcpSocket *socket, const QString &account, const QString &nickname, 
                       const QStringList &initialRoomList, QWidget *parent)
    : QMainWindow(parent)
    , socket(socket)
    , account(account)
    , nickname(nickname)
    , currentReply(nullptr)
{
    networkManager = new QNetworkAccessManager(this);
    
    // 从配置文件加载API密钥
    loadApiKey();
    
    setupUI();
    
    // 重新连接socket的readyRead信号到本窗口
    disconnect(socket, &QTcpSocket::readyRead, nullptr, nullptr);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
    
    // 显示初始房间列表
    if (!initialRoomList.isEmpty()) {
        roomManager->updateRoomList(initialRoomList);
    }
}

MainWindow::~MainWindow()
{
    if (socket) {
        socket->disconnectFromHost();
    }
}

void MainWindow::setupUI()
{
    setWindowTitle(QString("AI-ChatRoom - %1").arg(nickname));
    setMinimumSize(1000, 600);
    resize(1200, 700);
    
    setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1f22;
        }
        QSplitter::handle {
            background-color: #3c3f41;
            width: 2px;
        }
        QTabWidget::pane {
            border: 1px solid #5a5a5a;
            background-color: #2b2d30;
        }
        QTabBar::tab {
            background-color: #3c3f41;
            color: #d0d0d0;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
        }
        QTabBar::tab:selected {
            background-color: #2b2d30;
            color: #5ac2c6;
        }
        QTabBar::tab:hover {
            background-color: #4a4d50;
        }
    )");

    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *splitter = new QSplitter(Qt::Horizontal, this);

    // 左侧 - 聊天室管理
    roomManager = new RoomManager(this);
    roomManager->setMinimumWidth(200);
    roomManager->setMaximumWidth(280);
    splitter->addWidget(roomManager);

    // 中间 - 聊天标签页
    chatTabs = new QTabWidget(this);
    chatTabs->setTabsClosable(true);
    chatTabs->setMovable(true);
    chatTabs->setMinimumWidth(400);
    splitter->addWidget(chatTabs);

    // 右侧 - AI助手
    aiAssistant = new AIAssistant(this);
    aiAssistant->setMinimumWidth(250);
    aiAssistant->setMaximumWidth(350);
    splitter->addWidget(aiAssistant);

    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 0);

    mainLayout->addWidget(splitter);

    // 连接信号
    connect(roomManager, &RoomManager::createRoomRequested, this, &MainWindow::onRoomCreated);
    connect(roomManager, &RoomManager::joinRoomRequested, this, &MainWindow::onRoomJoined);
    connect(aiAssistant, &AIAssistant::aiRequestSent, this, &MainWindow::onAIRequest);
    connect(chatTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    connect(chatTabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

void MainWindow::onSocketReadyRead()
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

void MainWindow::handleMessage(const QJsonObject &obj)
{
    QString type = obj.value("type").toString();
    
    if (type == "room_list") {
        QStringList rooms;
        const QJsonArray arr = obj.value("rooms").toArray();
        for (const QJsonValue &val : arr) {
            rooms.append(val.toString());
        }
        roomManager->updateRoomList(rooms);
    }
    else if (type == "create_room_ok") {
        // Room created successfully, will join it next
    }
    else if (type == "create_room_fail") {
        QMessageBox::warning(this, "创建失败", obj.value("message").toString());
    }
    else if (type == "join_room_ok") {
        QString room = obj.value("room").toString();
        
        // Create new ChatWidget if not exists
        if (!chatWidgets.contains(room)) {
            ChatWidget *chatWidget = new ChatWidget(this);
            chatWidget->setRoomName(room);
            
            // Connect send message signal
            connect(chatWidget, &ChatWidget::sendMessageRequested, this, [this, room](const QString &msg) {
                QJsonObject obj;
                obj["type"] = "chat";
                obj["room"] = room;
                obj["message"] = msg;
                sendJson(obj);
            });
            
            chatWidgets[room] = chatWidget;
            chatWidget->setEnabled(true);  // Enable input
            
            // Add tab
            int index = chatTabs->addTab(chatWidget, room);
            chatTabs->setCurrentIndex(index);
        } else {
            // Switch to existing tab
            int index = chatTabs->indexOf(chatWidgets[room]);
            if (index >= 0) {
                chatTabs->setCurrentIndex(index);
            }
        }
        
        chatWidgets[room]->appendSystemMessage("成功加入聊天室: " + room);
        currentRoom = room;
        aiAssistant->setEnabled(true);
        aiAssistant->setChatContext(chatWidgets[room]->getChatHistory());
    }
    else if (type == "join_room_fail") {
        QMessageBox::warning(this, "加入失败", obj.value("message").toString());
    }
    else if (type == "chat") {
        QString room = obj.value("room").toString();
        QString from = obj.value("from").toString();
        QString message = obj.value("message").toString();
        QString time = obj.value("time").toString();
        
        // Route message to correct chat widget
        if (chatWidgets.contains(room)) {
            chatWidgets[room]->appendMessage(from, message, time);
            
            // Update AI context if this is current room
            if (room == currentRoom) {
                aiAssistant->setChatContext(chatWidgets[room]->getChatHistory());
            }
        }
    }
    else if (type == "system") {
        QString room = obj.value("room").toString();
        QString message = obj.value("message").toString();
        
        // Route system message to correct chat widget
        if (chatWidgets.contains(room)) {
            chatWidgets[room]->appendSystemMessage(message);
        }
    }
}

void MainWindow::sendJson(const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n');
    socket->write(data);
}

void MainWindow::onRoomCreated(const QString &roomName)
{
    QJsonObject obj;
    obj["type"] = "create_room";
    obj["room"] = roomName;
    sendJson(obj);
}

void MainWindow::onRoomJoined(const QString &roomName)
{
    QJsonObject obj;
    obj["type"] = "join_room";
    obj["room"] = roomName;
    sendJson(obj);
}

void MainWindow::onTabCloseRequested(int index)
{
    QString roomName = chatTabs->tabText(index);
    
    // Send leave_room message to server
    QJsonObject obj;
    obj["type"] = "leave_room";
    obj["room"] = roomName;
    sendJson(obj);
    
    // Remove tab and widget
    QWidget *widget = chatTabs->widget(index);
    chatTabs->removeTab(index);
    chatWidgets.remove(roomName);
    widget->deleteLater();
    
    // Update current room
    if (currentRoom == roomName) {
        if (chatTabs->count() > 0) {
            currentRoom = chatTabs->tabText(chatTabs->currentIndex());
            if (chatWidgets.contains(currentRoom)) {
                aiAssistant->setChatContext(chatWidgets[currentRoom]->getChatHistory());
            }
        } else {
            currentRoom.clear();
            aiAssistant->setEnabled(false);
        }
    }
}

void MainWindow::onTabChanged(int index)
{
    if (index >= 0) {
        currentRoom = chatTabs->tabText(index);
        if (chatWidgets.contains(currentRoom)) {
            aiAssistant->setChatContext(chatWidgets[currentRoom]->getChatHistory());
        }
    }
}

void MainWindow::onAIRequest(const QString &prompt)
{
    if (apiKey.isEmpty()) {
        QString configPath = getConfigFilePath();
        aiAssistant->appendResponse("❌ 错误: 未配置API密钥\n\n"
                                   "请在以下文件中配置你的 SiliconFlow API 密钥：\n" +
                                   configPath + "\n\n"
                                   "文件格式：\n"
                                   "SILICONFLOW_API_KEY=你的密钥\n\n"
                                   "配置后重启客户端即可");
        return;
    }
    
    callAI(prompt);
}

void MainWindow::callAI(const QString &prompt)
{
    // 如果有正在进行的请求，先取消
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
        currentReply = nullptr;
    }
    
    // 构建SiliconFlow API请求
    QUrl url("https://api.siliconflow.cn/v1/chat/completions");
    QNetworkRequest request(url);
    
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
    
    // 构建请求体
    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = prompt;
    
    QJsonArray messages;
    messages.append(messageObj);
    
    QJsonObject requestBody;
    // SiliconFlow模型，可以根据需要更改
    requestBody["model"] = "Qwen/Qwen2.5-7B-Instruct";
    requestBody["messages"] = messages;
    requestBody["temperature"] = 0.7;
    requestBody["max_tokens"] = 1000;
    
    QJsonDocument doc(requestBody);
    QByteArray data = doc.toJson();
    
    // 发送请求
    aiAssistant->appendResponse("⏳ 正在请求AI，请稍候...");
    currentReply = networkManager->post(request, data);
    
    // 连接响应信号
    connect(currentReply, &QNetworkReply::finished, this, &MainWindow::onAIReplyReceived);
}

void MainWindow::onAIReplyReceived()
{
    if (!currentReply) {
        return;
    }
    
    if (currentReply->error() == QNetworkReply::NoError) {
        QByteArray responseData = currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        QJsonObject obj = doc.object();
        
        if (obj.contains("choices")) {
            QJsonArray choices = obj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject firstChoice = choices[0].toObject();
                QJsonObject message = firstChoice["message"].toObject();
                QString content = message["content"].toString();
                
                aiAssistant->appendResponse("🤖 AI回复:\n\n" + content);
            } else {
                aiAssistant->appendResponse("❌ 错误: API返回为空");
            }
        } else if (obj.contains("error")) {
            QJsonObject error = obj["error"].toObject();
            QString errorMsg = error["message"].toString();
            aiAssistant->appendResponse("❌ API错误:\n\n" + errorMsg);
        } else {
            aiAssistant->appendResponse("❌ 错误: 无法解析API响应");
        }
    } else {
        QString errorString = currentReply->errorString();
        aiAssistant->appendResponse("❌ 网络错误:\n\n" + errorString + 
                                   "\n\n提示: 请检查网络连接和API密钥是否正确");
    }
    
    currentReply->deleteLater();
    currentReply = nullptr;
}

void MainWindow::loadApiKey()
{
    QString configPath = getConfigFilePath();
    QFile file(configPath);
    
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 配置文件不存在，创建示例文件
        QDir().mkpath(QFileInfo(configPath).absolutePath());
        
        QFile exampleFile(configPath);
        if (exampleFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&exampleFile);
            out << "# AI-ChatRoom API Configuration\n";
            out << "# Get your API key from: https://cloud.siliconflow.cn/\n";
            out << "# This file is ignored by git and won't be uploaded to GitHub\n\n";
            out << "SILICONFLOW_API_KEY=\n";
            exampleFile.close();
        }
        
        apiKey.clear();
        return;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        // 跳过注释和空行
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        
        // 解析 KEY=VALUE 格式
        if (line.startsWith("SILICONFLOW_API_KEY=")) {
            apiKey = line.mid(20).trimmed();  // 20是"SILICONFLOW_API_KEY="的长度
            break;
        }
    }
    file.close();
}

QString MainWindow::getConfigFilePath() const
{
    // 使用应用程序目录下的config文件夹
    QString appDir = QCoreApplication::applicationDirPath();
    return QDir(appDir).filePath("config/api.conf");
}
