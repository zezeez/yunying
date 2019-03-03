#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile qss(QStringLiteral(":/res/main.qss"));
    if (qss.open(QFile::ReadOnly)) {
        a.setStyleSheet(qss.readAll());
        qss.close();
    }
    MainWindow w;
    w.setWindowTitle(QString("12306 qt client"));
    w.setWindowIcon(QIcon(QStringLiteral(":/icon/images/ticket.ico")));
    w.show();

    return a.exec();
}
