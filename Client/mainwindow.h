#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTabWidget>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class RoomManager;
class ChatWidget;
class AIAssistant;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QTcpSocket *socket, const QString &account, const QString &nickname, 
                       const QStringList &initialRoomList = QStringList(), QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSocketReadyRead();
    void onRoomCreated(const QString &roomName);
    void onRoomJoined(const QString &roomName);
    void onAIRequest(const QString &prompt);
    void onTabCloseRequested(int index);
    void onTabChanged(int index);
    void onAIReplyReceived();

private:
    void handleMessage(const QJsonObject &obj);
    void sendJson(const QJsonObject &obj);
    void setupUI();
    void callAI(const QString &prompt);
    void loadApiKey();
    QString getConfigFilePath() const;

    QTcpSocket *socket;
    QString account;
    QString nickname;
    QString currentRoom;
    QByteArray buffer;

    RoomManager *roomManager;
    QTabWidget *chatTabs;
    QHash<QString, ChatWidget*> chatWidgets;
    AIAssistant *aiAssistant;
    
    QNetworkAccessManager *networkManager;
    QNetworkReply *currentReply;
    QString apiKey;  // API密钥从配置文件加载
};

#endif // MAINWINDOW_H
