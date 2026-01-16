#include "server.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QHostAddress>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("AI-ChatRoom-Server");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("AI-ChatRoom headless server");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption portOption(QStringList() << "p" << "port", "Server port", "port", "12345");
    parser.addOption(portOption);
    parser.process(a);

    bool ok = false;
    int port = parser.value(portOption).toInt(&ok);
    if (!ok || port <= 0 || port > 65535) {
        QTextStream(stderr) << "Invalid port. Using default 12345.\n";
        port = 12345;
    }

    Server server;
    server.Connect(port);

    QTextStream(stdout) << "AI-ChatRoom server listening on 0.0.0.0:" << port << "\n";
    return a.exec();
}
