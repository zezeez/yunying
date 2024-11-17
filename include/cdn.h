#ifndef CDN_H
#define CDN_H
#ifdef HAS_CDN
#include <QSslSocket>
#include <QVector>
#include <QQueue>
#include <QTimer>

class Cdn : public QObject
{
public:
    Cdn(QObject *parent = nullptr);
    ~Cdn();
    void setupSsl();
    void testCdnTimeout();
    void addCdn(const QString &cdn);
    void addCdns(const QStringList &cdnList);
    void addAvaliableCdn(const QString &cdn);
    void clear();
    void clearAvaliable();
    bool isEmpty();
    void startTest();
    void setMainCdn(const QString &cdn);
    void removeMainCdn();
    QString getMainCdn();
    QString getNextCdn();
    QString getCurCdn();
    QString getRandomCdn();
    void socketStateChanged();
    void socketEncrypted();
    void socketError();
    void sslError();
    void readyRead();

private:
    QSslSocket *ssl;
    QVector<QString> avaliableCdn;
    QQueue<QString> allCdn;
    int avaCdnIndex;
    QString mainCdn;
    QTimer testCdnTimer;
};
#endif
#endif // CDN_H
