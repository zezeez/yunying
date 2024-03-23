#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QSplashScreen>
#include <QScreen>
#include <QTimeZone>
#include <sysutil.h>
#include "nethelper.h"

#define _ QStringLiteral

MainWindow *w;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(QPixmap(QStringLiteral(":/icon/images/splash.jpeg")));
    splash->show();

    splash->showMessage(QObject::tr("正在加载资源..."),
                        Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    QFile qss(QStringLiteral(":/res/main.qss"));
    if (qss.open(QFile::ReadOnly)) {
        a.setStyleSheet(qss.readAll());
        qss.close();
    }
    splash->showMessage(QObject::tr("正在加载站点数据..."),
                        Qt::AlignBottom | Qt::AlignLeft, Qt::white);

    QCoreApplication::setOrganizationName(_("mpsoftware ltd.co"));
    QCoreApplication::setOrganizationDomain(_("mupiao.xyz"));
    QCoreApplication::setApplicationName(_("云映"));

    w = new MainWindow;
    w->setWindowTitle(QObject::tr("云映"));
    w->setWindowIcon(QIcon(_(":/icon/images/ticket.ico")));
    w->loadStationName();

    QList<QScreen *> screen = QGuiApplication::screens();
    if (!screen.isEmpty()) {
        QRect rect = screen[0]->geometry();
        w->move((rect.width() - w->width()) / 2, (rect.height() - w->height()) / 2);
    }

    splash->showMessage(QObject::tr("加载完成"),
                        Qt::AlignBottom | Qt::AlignLeft, Qt::white);
    splash->finish(w);
    delete splash;
    QString s1 = "2024-01-01";
    QString s2 = "2023-04-02";
    QString s3 = "2024-04-04";
    QVector<QString> v = { s1, s2, s3 };
    std::sort(v.begin(), v.end());
    qDebug() << v;

    //NetHelper::instance()->initLoginCookie();
    //NetHelper::instance()->getLoginConf();
    //NetHelper::instance()->isUamLogin();
    w->show();

    return a.exec();
}
