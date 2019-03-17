#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QSplashScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(QStringLiteral(":/icon/images/splash.jpeg")));
    splash->show();

    splash->showMessage(QObject::tr("Loading resource..."),
                        Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    QFile qss(QStringLiteral(":/res/main.qss"));
    if (qss.open(QFile::ReadOnly)) {
        a.setStyleSheet(qss.readAll());
        qss.close();
    }
    splash->showMessage(QObject::tr("Loading resource finished"),
                        Qt::AlignBottom | Qt::AlignLeft, Qt::white);

    MainWindow w;
    w.setWindowTitle(QString("12306 qt client"));
    w.setWindowIcon(QIcon(QStringLiteral(":/icon/images/ticket.ico")));

    splash->finish(&w);
    delete splash;

    w.show();

    return a.exec();
}
