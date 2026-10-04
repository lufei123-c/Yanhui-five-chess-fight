#include <QCoreApplication>
#include <QDebug>
#include "server.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    Server server;
    if (!server.start(8888)) {
        qDebug() << "服务端启动失败";
        return 1;
    }

    qDebug() << "按 Ctrl+C 退出...";
    return app.exec();
}
