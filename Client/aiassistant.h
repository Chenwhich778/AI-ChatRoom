#ifndef AIASSISTANT_H
#define AIASSISTANT_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

class AIAssistant : public QWidget
{
    Q_OBJECT
public:
    explicit AIAssistant(QWidget *parent = nullptr);

    void setChatContext(const QString &chatHistory);
    void appendResponse(const QString &response);
    void setEnabled(bool enabled);

signals:
    void aiRequestSent(const QString &prompt);

private slots:
    void onSendClicked();
    void onQuickActionClicked();

private:
    void setupUI();

    QTextEdit *responseDisplay;
    QLineEdit *promptEdit;
    QPushButton *sendButton;
    QPushButton *summarizeButton;
    QPushButton *suggestReplyButton;
    QComboBox *quickActions;

    QString chatContext;
};

#endif // AIASSISTANT_H
