#include "logindialog.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    LoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted) {
        MainWindow *mainWindow = new MainWindow(
            loginDialog.getSocket(),
            loginDialog.getAccount(),
            loginDialog.getNickname(),
            loginDialog.getRoomList()
        );
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainWindow->show();
        return a.exec();
    }
    
    return 0;
}
