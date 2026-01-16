#include "aiassistant.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>

AIAssistant::AIAssistant(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void AIAssistant::setupUI()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2d30;
        }
        QLabel {
            color: #d0d0d0;
            font-family: "Microsoft YaHei";
            font-size: 10pt;
        }
        QTextEdit {
            border: 1px solid #5a5a5a;
            border-radius: 6px;
            background-color: #3c3f41;
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
            border: 1px solid #9b59b6;
        }
        QPushButton {
            border: none;
            border-radius: 6px;
            background-color: #9b59b6;
            color: white;
            padding: 8px;
            font-family: "Microsoft YaHei";
            font-size: 9pt;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #8e44ad;
        }
        QPushButton:disabled {
            background-color: #555555;
            color: #888888;
        }
        QComboBox {
            border: 1px solid #5a5a5a;
            border-radius: 6px;
            background-color: #3c3f41;
            color: #d0d0d0;
            padding: 6px;
            font-family: "Microsoft YaHei";
        }
    )");

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    auto *titleLabel = new QLabel("AI 助手", this);
    titleLabel->setStyleSheet("font-size: 12pt; font-weight: bold; color: #9b59b6;");
    layout->addWidget(titleLabel);

    // 快捷操作
    auto *quickLabel = new QLabel("快捷操作:", this);
    layout->addWidget(quickLabel);

    auto *quickLayout = new QHBoxLayout();
    summarizeButton = new QPushButton("总结聊天", this);
    suggestReplyButton = new QPushButton("建议回复", this);
    quickLayout->addWidget(summarizeButton);
    quickLayout->addWidget(suggestReplyButton);
    layout->addLayout(quickLayout);

    // AI响应显示
    auto *responseLabel = new QLabel("AI 回复:", this);
    layout->addWidget(responseLabel);

    responseDisplay = new QTextEdit(this);
    responseDisplay->setReadOnly(true);
    responseDisplay->setPlaceholderText("AI回复将显示在这里...");
    layout->addWidget(responseDisplay, 1);

    // 自定义提示
    auto *promptLabel = new QLabel("自定义提问:", this);
    layout->addWidget(promptLabel);

    promptEdit = new QLineEdit(this);
    promptEdit->setPlaceholderText("输入你的问题...");
    layout->addWidget(promptEdit);

    sendButton = new QPushButton("发送给AI", this);
    layout->addWidget(sendButton);

    // 连接信号
    connect(sendButton, &QPushButton::clicked, this, &AIAssistant::onSendClicked);
    connect(promptEdit, &QLineEdit::returnPressed, this, &AIAssistant::onSendClicked);
    connect(summarizeButton, &QPushButton::clicked, this, [this]() {
        if (!chatContext.isEmpty()) {
            QString prompt = "请总结以下聊天内容的要点:\n\n" + chatContext;
            emit aiRequestSent(prompt);
            responseDisplay->setText("正在总结聊天内容...");
        } else {
            responseDisplay->setText("暂无聊天内容可总结");
        }
    });
    connect(suggestReplyButton, &QPushButton::clicked, this, [this]() {
        if (!chatContext.isEmpty()) {
            QString prompt = "基于以下聊天内容，建议我如何回复最后一条消息:\n\n" + chatContext;
            emit aiRequestSent(prompt);
            responseDisplay->setText("正在生成回复建议...");
        } else {
            responseDisplay->setText("暂无聊天内容");
        }
    });

    setEnabled(false);
}

void AIAssistant::setChatContext(const QString &chatHistory)
{
    chatContext = chatHistory;
}

void AIAssistant::appendResponse(const QString &response)
{
    responseDisplay->setText(response);
}

void AIAssistant::setEnabled(bool enabled)
{
    promptEdit->setEnabled(enabled);
    sendButton->setEnabled(enabled);
    summarizeButton->setEnabled(enabled);
    suggestReplyButton->setEnabled(enabled);
}

void AIAssistant::onSendClicked()
{
    QString prompt = promptEdit->text().trimmed();
    if (!prompt.isEmpty()) {
        QString fullPrompt = prompt;
        if (!chatContext.isEmpty()) {
            fullPrompt = "聊天上下文:\n" + chatContext + "\n\n用户问题: " + prompt;
        }
        emit aiRequestSent(fullPrompt);
        promptEdit->clear();
        responseDisplay->setText("正在等待AI回复...");
    }
}

void AIAssistant::onQuickActionClicked()
{
    // 预留给后续扩展
}
