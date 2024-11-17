#ifdef HAS_CDN
#include "cdn.h"
#include "userdata.h"
#include "mainwindow.h"
#include <QRandomGenerator>

#define _ QStringLiteral

extern MainWindow *w;

Cdn::Cdn(QObject *parent) : QObject(parent)
{
    avaCdnIndex = 0;
    connect(&testCdnTimer, &QTimer::timeout, this, &Cdn::testCdnTimeout);
}

Cdn::~Cdn()
{
    if (ssl) {
        delete ssl;
        ssl = nullptr;
    }
}

void Cdn::setupSsl()
{
    if (!ssl) {
        ssl = new QSslSocket;
    }

    connect(ssl, &QSslSocket::stateChanged,
            this, &Cdn::socketStateChanged);
    connect(ssl, &QSslSocket::encrypted,
            this, &Cdn::socketEncrypted);
    connect(ssl, &QSslSocket::errorOccurred,
            this, &Cdn::socketError);
    connect(ssl, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
            this, &Cdn::sslError);
    connect(ssl, &QSslSocket::readyRead,
            this, &Cdn::readyRead);
    /*connect(ssl, &QSslSocket::disconnected,
            this, &Cdn::startTest);*/

    ssl->setPeerVerifyName("kyfw.12306.cn");
}

void Cdn::testCdnTimeout()
{
    if (!allCdn.isEmpty()) {
        if (ssl->state() == QAbstractSocket::UnconnectedState) {
            startTest();
        } else {
            socketError();
        }
    } else {
        if (ssl) {
            delete ssl;
            ssl = nullptr;
        }
        testCdnTimer.stop();
    }
}

void Cdn::socketStateChanged()
{

}

void Cdn::socketEncrypted()
{
    if (!allCdn.isEmpty()) {
        avaliableCdn.append(allCdn.front());
        qDebug() << "add ava host: " << allCdn.front();
        w->updateAvaliableCdnNum(avaliableCdn.size());
        allCdn.pop_front();
    }
    ssl->disconnectFromHost();
}

void Cdn::socketError()
{
    if (!allCdn.isEmpty()) {
        allCdn.pop_front();
    }
    ssl->disconnectFromHost();
    ssl->close();
}

void Cdn::sslError()
{
    socketError();
}

void Cdn::readyRead()
{

}

void Cdn::addCdn(const QString &cdn)
{
    UserData *ud = UserData::instance();
    if (ud->generalSetting.cdnEnable) {
        allCdn.append(cdn);
    }
}

void Cdn::addCdns(const QStringList &cdnList)
{
    UserData *ud = UserData::instance();
    if (ud->generalSetting.cdnEnable) {
        allCdn.append(cdnList);
    }
}

void Cdn::addAvaliableCdn(const QString &cdn)
{
    UserData *ud = UserData::instance();
    if (ud->generalSetting.cdnEnable && !avaliableCdn.contains(cdn)) {
        avaliableCdn.append(cdn);
        w->updateAvaliableCdnNum(avaliableCdn.size());
    }
}

void Cdn::clear()
{
    allCdn.clear();
}

void Cdn::clearAvaliable()
{
    avaliableCdn.clear();
    avaCdnIndex = 0;
}

bool Cdn::isEmpty()
{
    return allCdn.isEmpty();
}

void Cdn::startTest()
{
    if (allCdn.isEmpty()) {
        return;
    }
    if (!ssl) {
        setupSsl();
    }
    QString host = allCdn.front();
    ssl->connectToHostEncrypted(host, 443);
    testCdnTimer.setInterval(2000);
    testCdnTimer.start();
}

void Cdn::setMainCdn(const QString &cdn)
{
    UserData *ud = UserData::instance();
    if (ud->generalSetting.cdnEnable) {
        mainCdn = cdn;
        qDebug() << "main cdn: " << cdn;
    }
}

void Cdn::removeMainCdn()
{
    mainCdn.clear();
}

QString Cdn::getMainCdn()
{
    UserData *ud = UserData::instance();
    if (!ud->generalSetting.cdnEnable) {
        return "";
    }
    return mainCdn;
}

QString Cdn::getNextCdn()
{
    UserData *ud = UserData::instance();
    if (avaliableCdn.isEmpty() || !ud->generalSetting.cdnEnable) {
        return "";
    }
    avaCdnIndex = avaCdnIndex < avaliableCdn.size() - 1 ? avaCdnIndex + 1 : 0;
    return avaliableCdn[avaCdnIndex];
}

QString Cdn::getCurCdn()
{
    UserData *ud = UserData::instance();
    if (avaliableCdn.isEmpty() || !ud->generalSetting.cdnEnable) {
        return "";
    }
    return avaliableCdn[avaCdnIndex];
}

QString Cdn::getRandomCdn()
{
    UserData *ud = UserData::instance();
    if (avaliableCdn.isEmpty() || !ud->generalSetting.cdnEnable) {
        return "";
    }
    std::uniform_int_distribution<int> dist(0, avaliableCdn.size() - 1);
    int idx = dist(*QRandomGenerator::global());
    return avaliableCdn[idx];
}

#endif
