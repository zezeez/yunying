#ifndef NETHELPER_H
#define NETHELPER_H

#include <QObject>
#include <QMap>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

class QNetworkReply;

enum ENETRPLYENUM {
    EGETVARIFICATIONCODE = 1,
    ELOGIN,
    EQUERYTICKET
};

class NetHelper : public QObject
{
    Q_OBJECT
public:
    ~NetHelper();
    static NetHelper *instance();
    void get(const QString &url);
    void getVarificationImage();
    void doLogin();
    void queryTicket(const QString &staStartCode, const QString &staEndCode, const QString &date);

signals:
    void finished(QNetworkReply *reply);
public slots:

private:
    explicit NetHelper(QObject *parent = nullptr);
    NetHelper(const NetHelper &) Q_DECL_EQ_DELETE;
    NetHelper &operator=(const NetHelper rhs) Q_DECL_EQ_DELETE;

    class NetHelperPrivate;
    friend class NetHelperPrivate;
public:
    NetHelperPrivate *d;
    QMap<QNetworkReply *, ENETRPLYENUM> replyMap;
};

class NetHelper::NetHelperPrivate
{
public:
    NetHelperPrivate(NetHelper *p) :
        man(new QNetworkAccessManager(p))
    {
    }

public:
    QNetworkAccessManager *man;
};

#endif // NETHELPER_H
