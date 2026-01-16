#include "chatwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ChatWidget::setupUI()
{
    setStyleSheet(R"(
        QWidget {
            background-color: #2b2d30;
        }
        QLabel {
            color: #5ac2c6;
            font-family: "Microsoft YaHei";
            font-size: 12pt;
            font-weight: bold;
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
            padding: 10px;
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
            padding: 10px 20px;
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
    )");

    auto *layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    roomLabel = new QLabel("请先加入聊天室", this);
    layout->addWidget(roomLabel);

    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    layout->addWidget(chatDisplay, 1);

    auto *inputLayout = new QHBoxLayout();
    messageEdit = new QLineEdit(this);
    messageEdit->setPlaceholderText("输入消息...");
    messageEdit->setEnabled(false);
    inputLayout->addWidget(messageEdit, 1);

    sendButton = new QPushButton("发送", this);
    sendButton->setEnabled(false);
    inputLayout->addWidget(sendButton);

    layout->addLayout(inputLayout);

    connect(sendButton, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
    connect(messageEdit, &QLineEdit::returnPressed, this, &ChatWidget::onSendClicked);
}

void ChatWidget::setRoomName(const QString &name)
{
    roomLabel->setText("聊天室: " + name);
}

void ChatWidget::appendMessage(const QString &from, const QString &message, const QString &time)
{
    QString formatted = QString("[%1] %2: %3").arg(time, from, message);
    chatDisplay->append(formatted);
    chatHistory += formatted + "\n";
}

void ChatWidget::appendSystemMessage(const QString &message)
{
    QString formatted = QString("<i style='color:#888;'>%1</i>").arg(message);
    chatDisplay->append(formatted);
    chatHistory += "[系统] " + message + "\n";
}

void ChatWidget::setEnabled(bool enabled)
{
    messageEdit->setEnabled(enabled);
    sendButton->setEnabled(enabled);
    if (enabled) {
        messageEdit->setFocus();
    }
}

void ChatWidget::clear()
{
    chatDisplay->clear();
    chatHistory.clear();
    roomLabel->setText("请先加入聊天室");
}

QString ChatWidget::getChatHistory() const
{
    return chatHistory;
}

void ChatWidget::onSendClicked()
{
    QString msg = messageEdit->text().trimmed();
    if (!msg.isEmpty()) {
        emit sendMessageRequested(msg);
        messageEdit->clear();
        messageEdit->setFocus();
    }
}
