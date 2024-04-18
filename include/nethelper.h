#ifndef NETHELPER_H
#define NETHELPER_H

#include <QObject>
#include <QMap>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QTimer>
#include <QList>
#include <QNetworkCookieJar>
#include <cookieapi.h>
#include <frozentrain.h>
#ifdef HAS_CDN
#include <cdn.h>
#endif

class QNetworkReply;

class ReqParam
{
public:
    void put(QString key, QString value);
    const QString &get() const;
    void finish();
    bool isEmpty() const;
private:
    QString data;
};

class NetHelper : public QObject
{
    Q_OBJECT
public:
    enum NETHELPERSTAT {
        ESUBMIT = 0,
        ESUBMITFAILED,
        ECANDIDATE,
        ECANDIDATEFAILED,
        ENETERR,
        EBADREPLY,
        ENETHELPERSTATMAX,
    };
    ~NetHelper();
    static NetHelper *instance();
    typedef void (NetHelper::*replyCallBack)(QNetworkReply *);
    inline void netStatInc(enum NETHELPERSTAT stat) {
        Q_ASSERT(stat < ENETHELPERSTATMAX);
        netStatSample[stat]++;
    }
    void getCookieHeader(QNetworkReply *reply);
    void setCookieHeader(const QUrl &url, QNetworkRequest &request);
    void setHeader(const QUrl &url, QNetworkRequest &request);
    void post(const QUrl &url, ReqParam &param, replyCallBack rcb);
    void post(const QUrl &url, ReqParam &param, replyCallBack rcb, QList<std::pair<QString, QString>> &headers);
    void anyPost(const QUrl &url, ReqParam &param, replyCallBack rcb);
    void get(const QUrl &url, replyCallBack rcb);
    void get(const QUrl &url, replyCallBack rcb, QList<std::pair<QString, QString>> &headers);
    void anyGet(const QUrl &url, replyCallBack rcb);
    void ignoreReply(QNetworkReply *reply);
    void initLoginCookie();
    void initLoginCookieReply(QNetworkReply *reply);
    void getLoginConf();
    void getLoginConfReply(QNetworkReply *reply);
    void onLogin();
    void onSmsVerificationLogin(const QString &verification_code);
    void onLoginUamReply(QNetworkReply *reply);
    void isUamLogin();
    int caculateRTTDelay(QNetworkReply *reply, enum QNetworkReply::NetworkError errorNo);
    void isUamLoginReply(QNetworkReply *reply);
    int checkReplyOk(QNetworkReply *reply);
    int replyIsOk(QNetworkReply *reply, QVariantMap &varMap);
    void loginForUam(ReqParam &param);
    void loginForUamReply(QNetworkReply *reply);
    void loginForLocationPasscode(ReqParam &param);
    void loginForLocation();
    void loginForLocationPasscodeReply(QNetworkReply *reply);
    void loginIndex();
    void loginSuccess();
    void logout();
    void logoutReply(QNetworkReply *reply);
    void createQrCode();
    void createQrCodeReply(QNetworkReply *reply);
    void checkQrCode(const QString &uuid);
    void checkQrCodeReply(QNetworkReply *reply);
    void sendSmsRequest(const QString &idCardNumTail);
    void sendSmsRequestReply(QNetworkReply *reply);
    void queryTicket();
    void queryTicketReply(QNetworkReply *reply);
    void queryDiffDateTicket(const QString &date);
    void queryDiffDateTicketReply(QNetworkReply *reply);
    void queryTrainStopStation(const QList<QString> &args);
    void queryTrainStopStationReply(QNetworkReply *reply);
    void passportUamtk();
    void passportUamtkReply(QNetworkReply *reply);
    void passportUamtkClient(const QString &apptk);
    void passportUamtkClientReply(QNetworkReply *reply);
    void userIsLogin();
    void getPassengerInfo();
    void getPassengerInfoReply(QNetworkReply *reply);
    void passengerInitDc();
    void checkUser();
    void checkUserReply(QNetworkReply *reply);
    void initDc();
    void initDcReply(QNetworkReply *reply);
    void initMy12306Api();
    void initMy12306ApiReply(QNetworkReply *reply);
    void handleError();
    void submitOrderRequest();
    void submitOrderRequestReply(QNetworkReply *reply);
    void checkOrderInfo();
    void checkOrderInfoReply(QNetworkReply *reply);
    void getQueueCount();
    void getQueueCountReply(QNetworkReply *reply);
    void confirmSingle();
    void confirmSingleReply(QNetworkReply *reply);
    // Async
    void confirmSingleForQueue();
    void confirmSingleForQueueReply(QNetworkReply *reply);
    void displayWaitTime();
    void queryOderWaitTime();
    void queryOderWaitTimeReply(QNetworkReply *reply);
    void resultOrderForDcQueue();
    void resultOrderForDcQueueReply(QNetworkReply *reply);
    void getStationNameTxt();
    void getStationNameTxtReply(QNetworkReply *);
    void saveStationNameFile(const QByteArray &nameText);
    void mayFrozenCurrentTrain(const QString &dispMsg);
    bool queryFrozenTrain(const QString &trainDesc);
    void grabTicketSuccess();
    void candidateTrain();
    void candidateTrainReply(QNetworkReply *reply);
    void showCandidateWarn(const QString &face_check_code, bool is_show_qrcode);
    void handlecandidateError();
    void candidateEntry(const struct CandidateDateInfo &dInfo);
    void chechFace(const struct CandidateDateInfo &dInfo);
    void checkFaceReply(QNetworkReply *reply);
    void submitCandidateOrderRequest();
    void submitCandidateOrderRequestReply(QNetworkReply *reply);
    void passengerInitApi();
    void passengerInitApiReply(QNetworkReply *reply);
    void getCandidateQueueNum();
    void getCandidateQueueNumReply(QNetworkReply *reply);
    QString generateCandidateTicketInfo();
    void confirmHB();
    void confirmHBReply(QNetworkReply *reply);
    void candidateQueryQueue();
    void candidateQueryQueueReply(QNetworkReply *reply);
    void sendMail();
    void sendCandidateMail();
    void lineUptoPayConfirm(const QString &reserve_no);

    // 微信通知
    void sendWxNotify(const QString &sendKey, const QString &msg);
    void sendWxNotifyReply(QNetworkReply *reply);
    void queryWxNotifyStatus(const QString &pushId, const QString &readKey);
    void queryWxNotifyStatusReply(QNetworkReply *reply);

    void checkUpdate();
    void checkUpdateReply(QNetworkReply *reply);
#ifdef HAS_CDN
    void getCdn();
    void getCdnReply(QNetworkReply *reply);
#endif

signals:
    void finished(QNetworkReply *reply);
public slots:

private:
    explicit NetHelper(QObject *parent = nullptr);
    NetHelper(const NetHelper &) Q_DECL_EQ_DELETE;
    NetHelper &operator=(const NetHelper rhs) Q_DECL_EQ_DELETE;

public:
    QNetworkAccessManager *nam;
    QMap<QNetworkReply *, replyCallBack> replyMap;
    QTimer *queryOrderTimer;
    QTimer *queryCandidateTimer;
    int orderWaitTime;
    int nextRequestOrderStatusTime;
    QVariantMap waitOrderInfo;
    CookieAPI *cookieJar;
    QMap<QNetworkReply *, qint64> rttMap;
    QVector<int> rttSamples;
    FrozenTrain frozenTrain;
    QString queryLeftTicketUrl;
    QString chooseSeat;
    bool canChooseSeats;
    QVector<int> netStatSample;
    QVector<QString> cdnList;
    QVector<QVector<int>> statSnapshot;
    QTimer *capSnapTimer;
    QTimer *rttDelayTimer;
#ifdef HAS_CDN
    Cdn cdn;
#endif
};


#endif // NETHELPER_H
