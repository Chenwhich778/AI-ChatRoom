#include "roommanager.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>

RoomManager::RoomManager(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void RoomManager::setupUI()
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
        QListWidget {
            border: 1px solid #5a5a5a;
            border-radius: 6px;
            background-color: #3c3f41;
            color: #d0d0d0;
            font-family: "Microsoft YaHei";
            font-size: 10pt;
        }
        QListWidget::item:selected {
            background-color: #5ac2c6;
            color: white;
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
        QPushButton {
            border: none;
            border-radius: 6px;
            background-color: #5ac2c6;
            color: white;
            padding: 8px;
            font-family: "Microsoft YaHei";
            font-size: 9pt;
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

    auto *titleLabel = new QLabel("聊天室管理", this);
    titleLabel->setStyleSheet("font-size: 12pt; font-weight: bold; color: #5ac2c6;");
    layout->addWidget(titleLabel);

    auto *listLabel = new QLabel("可用聊天室:", this);
    layout->addWidget(listLabel);

    roomList = new QListWidget(this);
    roomList->setMaximumHeight(150);
    layout->addWidget(roomList);

    joinButton = new QPushButton("加入选中聊天室", this);
    layout->addWidget(joinButton);

    layout->addSpacing(10);

    auto *createLabel = new QLabel("创建新聊天室:", this);
    layout->addWidget(createLabel);

    newRoomEdit = new QLineEdit(this);
    newRoomEdit->setPlaceholderText("输入聊天室名称");
    layout->addWidget(newRoomEdit);

    createButton = new QPushButton("创建", this);
    layout->addWidget(createButton);

    layout->addStretch();

    connect(createButton, &QPushButton::clicked, this, &RoomManager::onCreateClicked);
    connect(joinButton, &QPushButton::clicked, this, &RoomManager::onJoinClicked);
    connect(roomList, &QListWidget::itemDoubleClicked, this, &RoomManager::onJoinClicked);
    connect(newRoomEdit, &QLineEdit::returnPressed, this, &RoomManager::onCreateClicked);
}

void RoomManager::updateRoomList(const QStringList &rooms)
{
    roomList->clear();
    for (const QString &room : rooms) {
        roomList->addItem(room);
    }
}

void RoomManager::setEnabled(bool enabled)
{
    roomList->setEnabled(enabled);
    newRoomEdit->setEnabled(enabled);
    createButton->setEnabled(enabled);
    joinButton->setEnabled(enabled);
}

void RoomManager::onCreateClicked()
{
    QString name = newRoomEdit->text().trimmed();
    if (!name.isEmpty()) {
        emit createRoomRequested(name);
        newRoomEdit->clear();
    }
}

void RoomManager::onJoinClicked()
{
    auto *item = roomList->currentItem();
    if (item) {
        emit joinRoomRequested(item->text());
    }
}
