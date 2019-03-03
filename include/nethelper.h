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
    EDOVARIFICATION,
    ELOGIN,
    EQUERYTICKET,
    EGETSTATIONNAMETXT,
    EPASSPORTUAMTK,
    EPASSPORTUAMTKCLIENT,
    EQUERYLOGINSTATUS,
};

#define BASEURL "https://kyfw.12306.cn"
#define PASSPORTURL "https://kyfw.12306.cn/passport"
#define LOGINURL "https://kyfw.12306.cn/passport/web/login"
#define TICKETURL "https://kyfw.12306.cn/otn/leftTicket"
#define CAPTCHACHECKURL "https://kyfw.12306.cn/passport/captcha/captcha-check"
#define CAPTCHAIMAGEURL "https://kyfw.12306.cn/passport/captcha/captcha-image"
#define USERAGENT "Mozilla/5.0 (X11; Fedora; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36"

class NetHelper : public QObject
{
    Q_OBJECT
public:
    ~NetHelper();
    static NetHelper *instance();
    void get(const QString &url);
    void getVarificationImage();
    void doVarification(QVector<QPoint> points);
    void doLogin(QVector<QPoint> points, QString name, QString passwd);
    void queryTicket(const QString &staStartCode, const QString &staEndCode, const QString &date);
    void getStationNameTxt();
    void passportUamtk();
    void passportUamtkClient(QString apptk);
    void userIsLogin();

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
