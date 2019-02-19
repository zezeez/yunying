#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QString("12306 qt client"));
    w.setWindowIcon(QIcon(QStringLiteral(":/icon/images/ticket.ico")));
    w.show();

    return a.exec();
}
