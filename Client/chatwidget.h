#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class ChatWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWidget(QWidget *parent = nullptr);

    void setRoomName(const QString &name);
    void appendMessage(const QString &from, const QString &message, const QString &time);
    void appendSystemMessage(const QString &message);
    void setEnabled(bool enabled);
    void clear();
    QString getChatHistory() const;

signals:
    void sendMessageRequested(const QString &message);

private slots:
    void onSendClicked();

private:
    void setupUI();

    QLabel *roomLabel;
    QTextEdit *chatDisplay;
    QLineEdit *messageEdit;
    QPushButton *sendButton;
    QString chatHistory;
};

#endif // CHATWIDGET_H
