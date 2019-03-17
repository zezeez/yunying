#ifndef NETHELPER_H
#define NETHELPER_H

#include <QObject>
#include <QMap>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

class QNetworkReply;

enum NETREQUESTTYPEENUM {
    EGETVARIFICATIONCODE = 1,
    EDOVARIFICATION,
    ELOGIN,
    EQUERYTICKET,
    EGETSTATIONNAMETXT,
    EPASSPORTUAMTK,
    EPASSPORTUAMTKCLIENT,
    EQUERYLOGINSTATUS,
    EGETPASSENGERINFO,
    EPASSENGERINITDC,
    ECHECKUSER,
    ESUBMITORDERREQUEST,
    ECHECKORDERINFO,
};

#define BASEURL "https://kyfw.12306.cn"
#define OTNURL "https://kyfw.12306.cn/otn"
#define PASSPORTURL "https://kyfw.12306.cn/passport"
#define LOGINURL "https://kyfw.12306.cn/passport/web/login"
#define TICKETURL "https://kyfw.12306.cn/otn/leftTicket"
#define CAPTCHACHECKURL "https://kyfw.12306.cn/passport/captcha/captcha-check"
#define CAPTCHAIMAGEURL "https://kyfw.12306.cn/passport/captcha/captcha-image"
#define USERAGENT "Mozilla/5.0 (X11; Fedora; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36"

typedef void (*replyCallBack)(QNetworkReply *);

class NetHelper : public QObject
{
    Q_OBJECT
public:
    ~NetHelper();
    static NetHelper *instance();
    void post(const QUrl &url, const QString &data, enum NETREQUESTTYPEENUM type);
    void get(const QUrl &url, enum NETREQUESTTYPEENUM type);
    void getVarificationImage();
    void doVarification(QVector<QPoint> points);
    void doLogin(QVector<QPoint> points, QString name, QString passwd);
    void queryTicket(const QString &staStartCode, const QString &staEndCode, const QString &date);
    void getStationNameTxt();
    void passportUamtk();
    void passportUamtkClient(QString apptk);
    void userIsLogin();
    void getPassengerInfo();
    void passengerInitDc();
    void checkUser();
    void submitOrderRequest(const QString &secStr, const QString &date,
                            const QString fromStation, const QString toStation);
    void checkOrderInfo(const QString &passenger, const QString &oldPassenger);

signals:
    void finished(QNetworkReply *reply);
public slots:

private:
    explicit NetHelper(QObject *parent = nullptr);
    NetHelper(const NetHelper &) Q_DECL_EQ_DELETE;
    NetHelper &operator=(const NetHelper rhs) Q_DECL_EQ_DELETE;

public:
    QNetworkAccessManager *nam;
    QMap<QNetworkReply *, NETREQUESTTYPEENUM> replyMap;
};


#endif // NETHELPER_H
