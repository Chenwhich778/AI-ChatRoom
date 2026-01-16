#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>

class RoomManager : public QWidget
{
    Q_OBJECT
public:
    explicit RoomManager(QWidget *parent = nullptr);

    void updateRoomList(const QStringList &rooms);
    void setEnabled(bool enabled);

signals:
    void createRoomRequested(const QString &roomName);
    void joinRoomRequested(const QString &roomName);

private slots:
    void onCreateClicked();
    void onJoinClicked();

private:
    void setupUI();

    QListWidget *roomList;
    QLineEdit *newRoomEdit;
    QPushButton *createButton;
    QPushButton *joinButton;
};

#endif // ROOMMANAGER_H
