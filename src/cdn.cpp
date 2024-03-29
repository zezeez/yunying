#ifdef HAS_CDN
#include "cdn.h"

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
    if (allCdn.isEmpty())
        return;
    avaliableCdn.append(allCdn.front());
    qDebug() << "add ava host: " << allCdn.front();
    allCdn.pop_front();
    ssl->disconnectFromHost();
}

void Cdn::socketError()
{
    if (allCdn.isEmpty())
        return;
    allCdn.pop_front();
    ssl->disconnectFromHost();
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
    allCdn.append(cdn);
}

void Cdn::addCdns(const QStringList &cdnList)
{
    allCdn.append(cdnList);
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
    mainCdn = cdn;
}

void Cdn::removeMainCdn()
{
    mainCdn.clear();
}

QString Cdn::getMainCdn()
{
    return mainCdn;
}

QString Cdn::getNextCdn()
{
    if (avaliableCdn.isEmpty()) {
        return "";
    }
    avaCdnIndex = avaCdnIndex < avaliableCdn.size() - 1 ? avaCdnIndex + 1 : 0;
    return avaliableCdn[avaCdnIndex];
}

QString Cdn::getCurCdn()
{
    if (avaliableCdn.isEmpty()) {
        return "";
    }
    return avaliableCdn[avaCdnIndex];
}
#endif
