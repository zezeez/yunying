#include "nethelper.h"
#include "mainwindow.h"
#include "userdata.h"
#include "12306.h"
#include "lib/sm4/include/sm4.h"
#include "loginconf.h"
#include "logindialog.h"
#include "analysis.h"
#include "seatdialog.h"
#include "version.h"
#include "lib/base64/include/base64.h"
#include <QRandomGenerator>
#include <QPoint>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QLayout>
#include <QMessageBox>
#include <QTimer>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QTimeZone>
#include <QMovie>
#include <QDesktopServices>
#include <QDomDocument>
#include <QTemporaryFile>

#define REQUESTTIMEOUT 10000
#define _ QStringLiteral

extern MainWindow *w;

void ReqParam::put(const QString &key, const QString &value)
{
    data.append(_("%1=%2&").arg(key, value));
}

const QString &ReqParam::get() const
{
    return data;
}

void ReqParam::finish()
{
    if (data.length())
        data.truncate(data.length() - 1);
}

bool ReqParam::isEmpty() const
{
    return data.isEmpty();
}

NetHelper::NetHelper(QObject *parent) : QObject(parent),
    nam(new QNetworkAccessManager(this)),
    cookieJar(new CookieAPI),
    nam2(new QNetworkAccessManager(this)),
    cookieJar2(new QNetworkCookieJar)
{
    /*connect(nam, &QNetworkAccessManager::finished,
                this, &NetHelper::finished);*/

    connect(nam, &QNetworkAccessManager::finished, this, &NetHelper::networkReplyHandle);
    nam->setCookieJar(cookieJar);
    connect(nam2, &QNetworkAccessManager::finished, this, &NetHelper::networkReplyHandle);
    nam2->setCookieJar(cookieJar2);

    queryLeftTicketUrl = _(QUERYTICKETBASEURL);

    netStatSample.fill(0, ENETHELPERSTATMAX);
    statSnapshot.resize(2);
    statSnapshot[0].fill(0, ENETHELPERSTATMAX);
    statSnapshot[1].fill(0, ENETHELPERSTATMAX);
    capSnapTimer = new QTimer;
    connect(capSnapTimer, &QTimer::timeout, this, [this] () {
        statSnapshot[1] = statSnapshot[0];
        statSnapshot[0] = netStatSample;
        QVector<int> v;
        for (int i = 0; i < statSnapshot[0].size(); i++) {
            v.append(statSnapshot[0][i] - statSnapshot[1][i]);
        }
        w->statChart->update(v);
    });
    capSnapTimer->setInterval(60 * 60 * 1000);
    capSnapTimer->start();
    rttDelayTimer = new QTimer;
    connect(rttDelayTimer, &QTimer::timeout, this, [this] () {
        int total = 0;
        if (rttSamples.isEmpty()) {
            return;
        }
        for (auto &d : rttSamples) {
            total += d;
        }
        total /= rttSamples.size();
        w->delayChart->update(total);
        rttSamples.clear();
    });
    rttDelayTimer->setInterval(30 * 60 * 1000);
    rttDelayTimer->start();
    keepLoginTimer = new QTimer;
    connect(keepLoginTimer, &QTimer::timeout, this, &NetHelper::keepLogin);
    keepLoginTimer->setInterval(5 * 60 * 1000);

#ifdef HAS_CDN
    sip.dnsAnswer("kyfw.12306.cn");
#endif
}

void NetHelper::networkReplyHandle(QNetworkReply *reply)
{
    replyCallBack rcb = replyMap.value(reply);
    (this->*rcb)(reply);
    replyMap.remove(reply);
    reply->deleteLater();
}

NetHelper *NetHelper::instance()
{
    static NetHelper nh;
    return &nh;
}

#if 0
QList<QNetworkCookie> NetHelper::getCookieHeader(QNetworkReply *reply)
{
    QList<QNetworkCookie> cookieList;
    if (reply->hasRawHeader("Set-Cookie")) {
        QByteArray cookieHeader = reply->rawHeader("Set-Cookie");
        cookieList = QNetworkCookie::parseCookies(cookieHeader);
    }
    return cookieList;
}
#endif

void NetHelper::commonSetCookieHeader(QList<QNetworkCookie> &cookieList, QNetworkRequest &request)
{
    QByteArray cookieHeader;
    for (auto &i : cookieList) {
        cookieHeader.append(i.toRawForm(QNetworkCookie::NameAndValueOnly) + QStringLiteral("; ").toLatin1());
    }

    if (cookieHeader.size() > 2) {
        cookieHeader.resize(cookieHeader.size() - 2);
        request.setRawHeader("Cookie", cookieHeader);
        //qDebug() << "cookie: " << cookieHeader;
    }
}

void NetHelper::setCookieHeader(const QUrl &url, QNetworkRequest &request)
{
    QList<QNetworkCookie> cookieList = cookieJar->cookiesForUrl(url);
    if (!cookieList.empty()) {
        commonSetCookieHeader(cookieList, request);
    }
}

void NetHelper::setCookieHeader2(const QUrl &url, QNetworkRequest &request)
{
    QList<QNetworkCookie> cookieList = cookieJar2->cookiesForUrl(url);
    if (!cookieList.empty()) {
        commonSetCookieHeader(cookieList, request);
    }
}

void NetHelper::setHeader(const QUrl &url, QNetworkRequest &request)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, _("application/x-www-form-urlencoded; charset=UTF-8"));
    request.setRawHeader("User-Agent", USERAGENT);
    setCookieHeader(url, request);
}

void NetHelper::setHeader2(const QUrl &url, QNetworkRequest &request)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, _("application/x-www-form-urlencoded; charset=UTF-8"));
    request.setRawHeader("User-Agent", USERAGENT);
    setCookieHeader2(url, request);
}

void NetHelper::post(const QUrl &url, ReqParam &param, NetHelper::replyCallBack rcb, QList<std::pair<QString, QString>> &headers)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setTransferTimeout(REQUESTTIMEOUT);
#ifdef HAS_CDN
    if (url.host() == _("kyfw.12306.cn")) {
        QString mainCdn = cdn.getMainCdn();
        if (!mainCdn.isEmpty()) {
            request.setIpAddress(mainCdn);
            qDebug() << "main cdn: " << mainCdn;
        }
    }

    request.setPeerVerifyName(url.host());
#endif
    setHeader(url, request);
    QList<std::pair<QString, QString>>::const_iterator it;
    for (it = headers.cbegin(); it != headers.cend(); ++it) {
        request.setRawHeader(it->first.toUtf8(), it->second.toUtf8());
    }
    param.finish();
    QNetworkReply *reply = nam->post(request, param.get().toUtf8());
    replyMap.insert(reply, rcb);
    QDateTime time = QDateTime::currentDateTime();
    rttMap.insert(reply, time.toMSecsSinceEpoch());
}

void NetHelper::post(const QUrl &url, ReqParam &param, NetHelper::replyCallBack rcb)
{
    QList<std::pair<QString, QString>> headers;
    post(url, param, rcb, headers);
}

void NetHelper::anyPost(const QUrl &url, ReqParam &param, NetHelper::replyCallBack rcb)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setTransferTimeout(REQUESTTIMEOUT);
    request.setHeader(QNetworkRequest::ContentTypeHeader, _("application/x-www-form-urlencoded; charset=UTF-8"));
    request.setRawHeader("User-Agent", USERAGENT);
    param.finish();
    QNetworkReply *reply = nam->post(request, param.get().toUtf8());
    replyMap.insert(reply, rcb);
    QDateTime time = QDateTime::currentDateTime();
    rttMap.insert(reply, time.toMSecsSinceEpoch());
}

void NetHelper::get(const QUrl &url, replyCallBack rcb, QList<std::pair<QString, QString>> &headers)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setTransferTimeout(REQUESTTIMEOUT);
#ifdef HAS_CDN
    if (url.host() == _("kyfw.12306.cn")) {
        QString mainCdn = cdn.getMainCdn();
        if (!mainCdn.isEmpty()) {
            request.setIpAddress(mainCdn);
        }
    }
    request.setPeerVerifyName(url.host());
    qDebug() << request.ipAddress();
#endif
    setHeader(url, request);
    QList<std::pair<QString, QString>>::const_iterator it;
    for (it = headers.cbegin(); it != headers.cend(); ++it) {
        request.setRawHeader(it->first.toLatin1(), it->second.toLatin1());
    }
    QNetworkReply *reply = nam->get(request);
    replyMap.insert(reply, rcb);
    QDateTime time = QDateTime::currentDateTime();
    rttMap.insert(reply, time.toMSecsSinceEpoch());
}

void NetHelper::get(const QUrl &url, replyCallBack rcb)
{
    QList<std::pair<QString, QString>> headers;
    get(url, rcb, headers);
}

void NetHelper::get2(const QUrl &url, replyCallBack rcb, QList<std::pair<QString, QString>> &headers)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setTransferTimeout(REQUESTTIMEOUT);
#ifdef HAS_CDN
    static int reqCount;
    if (reqCount++ < 16) {
        request.setIpAddress(cdn.getCurCdn());
    } else {
        QString cur = cdn.getCurCdn();
        QString next = cdn.getNextCdn();
        if (!next.isEmpty()) {
            request.setIpAddress(next);
            if (cur != next) {
                nam2->clearConnectionCache();
            }
        }
        reqCount = 0;
    }
    request.setPeerVerifyName(url.host());
    qDebug() << request.ipAddress();
#endif
    setHeader2(url, request);
    QList<std::pair<QString, QString>>::const_iterator it;
    for (it = headers.cbegin(); it != headers.cend(); ++it) {
        request.setRawHeader(it->first.toLatin1(), it->second.toLatin1());
    }
    QNetworkReply *reply = nam2->get(request);
    replyMap.insert(reply, rcb);
    QDateTime time = QDateTime::currentDateTime();
    rttMap.insert(reply, time.toMSecsSinceEpoch());
}

void NetHelper::get2(const QUrl &url, replyCallBack rcb)
{
    QList<std::pair<QString, QString>> headers;
    get2(url, rcb, headers);
}

void NetHelper::anyGet(const QUrl &url, replyCallBack rcb)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setTransferTimeout(REQUESTTIMEOUT);
    request.setHeader(QNetworkRequest::ContentTypeHeader, _("application/x-www-form-urlencoded; charset=UTF-8"));
    request.setRawHeader("User-Agent", USERAGENT);
    QNetworkReply *reply = nam->get(request);
    replyMap.insert(reply, rcb);
    QDateTime time = QDateTime::currentDateTime();
    rttMap.insert(reply, time.toMSecsSinceEpoch());
}

int NetHelper::caculateRTTDelay(QNetworkReply *reply, enum QNetworkReply::NetworkError errorNo)
{
    qint64 sample = rttMap.value(reply);
    rttMap.remove(reply);
    if (reply->request().url().host() != _("kyfw.12306.cn")) {
        return -1;
    }
    if (errorNo != QNetworkReply::NoError) {
        w->updateNetQualityStatus(-1);
        //rttSamples.clear();
        return -1;
    }
    QDateTime time = QDateTime::currentDateTime();
    int rttDelay = time.toMSecsSinceEpoch() - sample;
    rttSamples.append(rttDelay);
    /*int size = rttSamples.size();

    if (size > 3) {
        int caculSize = size;
        for (int i = 0; i < size; i++) {
            if (rttSamples[i] == -1) {
                caculSize--;
                continue;
            }
            rttDelay += rttSamples[i];
        }
        rttDelay /= caculSize;
        rttDelay >>= 1;
        w->updateNetQualityStatus(rttDelay);
        rttSamples.clear();
    }*/
    w->updateNetQualityStatus(rttDelay);
    return rttDelay;
}

int NetHelper::checkReplyOk(QNetworkReply *reply)
{
    enum QNetworkReply::NetworkError errorNo = reply->error();

    if (errorNo != QNetworkReply::NoError) {
        w->formatOutput(QStringLiteral("%1: 请求响应异常，错误号：%2，错误详情：%3")
                            .arg(reply->request().url().url()).arg(errorNo).arg(reply->errorString()));
        if (errorNo == QNetworkReply::HostNotFoundError ||
            errorNo == QNetworkReply::TimeoutError ||
            errorNo == QNetworkReply::UnknownNetworkError) {
            w->formatOutput(_("网络异常，请检查网络连接！"));
            netStatInc(ENETERR);
        }
        caculateRTTDelay(reply, errorNo);
        return -1;
    }
    caculateRTTDelay(reply, errorNo);

    QVariant statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    int statusCodeInt = statusCode.toInt();
    if (statusCodeInt == 301 || statusCodeInt == 302)
    {
        // The target URL if it was a redirect:
        QString redirectionTargetUrl =
            reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        QUrl url(redirectionTargetUrl);
        get(url, replyMap.value(reply));
        qDebug() << "redirect to " << redirectionTargetUrl;
    } else if (statusCodeInt != 200) {
        w->formatOutput(QStringLiteral("服务器HTTP状态码返回错误(%1)").arg(statusCodeInt));
        netStatInc(EBADREPLY);
        return -1;
    }

    return 0;
}

int NetHelper::replyIsOk(QNetworkReply *reply, QVariantMap &varMap)
{
    if (checkReplyOk(reply) < 0)
        return -1;

    QJsonParseError error;
    QByteArray content = reply->readAll();
    QJsonDocument jsonDoc;

    QByteArray contentType = reply->rawHeader(_("Content-Type").toLatin1());

    if (contentType.startsWith("application/json")) {
        jsonDoc = QJsonDocument::fromJson(content, &error);
        if (error.error != QJsonParseError::NoError || jsonDoc.isNull()) {
            w->formatOutput(QStringLiteral("服务器返回数据JSON解析错误，错误位置：%1，错误码：%2，错误描述：%3")
                                .arg(error.offset).arg(error.error).arg(error.errorString()));
            qDebug() << content;
            netStatInc(EBADREPLY);
            return -1;
        }
    } else {
        //w->formatOutput(QStringLiteral("服务器返回错误"));
        varMap.insert(_("data"), content);
        varMap.insert(_("Content-Type"), contentType);
        QString s = QString::fromUtf8(content);
        if (s.contains(_("网络可能存在问题，请您重试一下"))) {
            w->formatOutput(_("服务器应答异常，可能此设备已被暂时限制访问"));
        }
        netStatInc(EBADREPLY);
        return -1;
    }

    varMap = jsonDoc.toVariant().toMap();
    return 0;
}

// Only care about Set-Cookie header
void NetHelper::ignoreReply(QNetworkReply *reply)
{
    checkReplyOk(reply);
}

void NetHelper::leftTicketInit()
{
    QUrl url(_(LEFTTICKETINIT));
    get2(url, &NetHelper::leftTicketInitReply);
}

void NetHelper::leftTicketInitReply(QNetworkReply *reply)
{
    if (checkReplyOk(reply) < 0)
        return;
}

void NetHelper::initLoginCookie()
{
    QUrl url = QStringLiteral(INITLOGINCOOKIE);

    get(url, &NetHelper::initLoginCookieReply);
}

void NetHelper::initLoginCookieReply(QNetworkReply *reply)
{
    if (checkReplyOk(reply) < 0)
        return;
}

void NetHelper::getLoginConf()
{
    ReqParam param;
    QUrl url = QStringLiteral(LOGINCONFURL);

    post(url, param, &NetHelper::getLoginConfReply);
}

void NetHelper::getLoginConfReply(QNetworkReply *reply)
{
    LoginConf &lconf = LoginConf::instance();
    QVariantMap response;

    if (replyIsOk(reply, response) < 0 )
        return;

    QVariantMap dataMap = response[_("data")].toMap();
    if (dataMap.isEmpty())
        return;
    QString value = dataMap[_("is_uam_login")].toString();
    //data.
    //QString value = data[_("is_uam_login")].toString();
    lconf.isUamLogin = value == _("Y");
    value = dataMap[_("is_login_passCode")].toString();
    lconf.isLoginPasscode = value == _("Y");
    value = dataMap[_("is_sweep_login")].toString();
    lconf.isSweepLogin = value == _("Y");
    value = dataMap[_("is_login")].toString();
    lconf.isLogin = value == _("Y");
    value = dataMap[_("is_message_passCode")].toString();
    lconf.isMessagePasscode =  value.length() == 0 || value == _("Y") ? true : false;
    value = dataMap[_("queryUrl")].toString();
    if (!value.isEmpty()) {
        queryLeftTicketUrl = _(BASEURL PUBLICNAME) + '/' + value;
        qDebug() << "new left ticket query url: " << value;
    }

    if (lconf.isUamLogin) {
        isUamLogin();
    } else {
        if (lconf.isLogin) {
            w->showMainWindow();
        } else {
            // 隐藏扫码登录入口
            w->loginDialog->hideQrCodeTab();
            // 本地登录
            /*if (lconf.isMessagePasscode) {
                // 显示验证方式
                w->loginDialog->showSmsVerification();
            }*/
        }
        keepLoginTimer->stop();
    }
}

void NetHelper::onLogin()
{
    QUrl url = QStringLiteral(CHECK_LOGIN_PASSPORT_URL);
    ReqParam param;
    UserData *ud = UserData::instance();
#ifdef HAS_CDN
    QString h = cdn.getRandomCdn();
    if (!h.isEmpty()) {
        cdn.setMainCdn(h);
    }
#endif
    LoginConf loginConf = LoginConf::instance();
    if (loginConf.isUamLogin) {
        param.put(_("username"), ud->getUserLoginInfo().userName);
        param.put(_("appid"), _(PASSPORT_APPID));
        post(url, param, &NetHelper::onLoginUamReply);
    } else {
        loginForLocation();
    }
}

void NetHelper::onSmsVerificationLogin(const QString &verification_code)
{
    ReqParam param;
    QString encode_passwd;
    UserData *ud = UserData::instance();
    param.put(_("sessionId"), _(""));
    param.put(_("sig"), _(""));
    param.put(_("if_check_slide_passcode_token"), _(""));
    param.put(_("scene"), _(""));
    param.put(_("checkMode"), "0");
    param.put(_("randCode"), verification_code);
    param.put(_("username"), ud->getUserLoginInfo().userName.toUtf8().toPercentEncoding());
    encode_passwd = sm4_encrypt_ecb(ud->getUserLoginInfo().passwd, SM4_KEY_SECRET);
    param.put(_("password"), '@' + encode_passwd.toUtf8().toPercentEncoding());
    param.put(_("appid"), _(PASSPORT_APPID));

    LoginConf &loginConf = LoginConf::instance();
    if (loginConf.isUamLogin) {
        // 统一认证登录+短信验证
        loginForUam(param);
    } else {
        //本地登录+短信验证
        loginForLocationPasscode(param);
    }
}

void NetHelper::onLoginUamReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    ReqParam param;
    UserData *ud = UserData::instance();
    QString encode_passwd;
    if (replyIsOk(reply, varMap) < 0)
        return;
    int result_code = varMap[_("result_code")].toInt();
    if (result_code)
        return;
    int login_check_code = varMap[_("login_check_code")].toInt();
    switch (login_check_code) {
    case 0:
    case 1:
    case 2:
        param.put(_("sessionId"), _(""));
        param.put(_("sig"), _(""));
        param.put(_("if_check_slide_passcode_token"), _(""));
        param.put(_("scene"), _(""));
        param.put(_("randCode"), _(""));
        param.put(_("username"), ud->getUserLoginInfo().userName.toUtf8().toPercentEncoding());
        encode_passwd = sm4_encrypt_ecb(ud->getUserLoginInfo().passwd, SM4_KEY_SECRET);
        param.put(_("password"), '@' + encode_passwd.toUtf8().toPercentEncoding());
        param.put(_("appid"), _(PASSPORT_APPID));
        loginForUam(param);
        break;
    // 短信验证
    case 3:
        w->loginDialog->hideUserNamePasswd();
        w->loginDialog->showSmsVerification();
        break;
    default:
        break;
    }
}

void NetHelper::isUamLogin()
{
    ReqParam data;
    QUrl url(QStringLiteral(PASSPORT_APPTK_STATIC));

    data.put(_("appid"), _(PASSPORT_APPID));
    post(url, data, &NetHelper::isUamLoginReply);
}

void NetHelper::isUamLoginReply(QNetworkReply *reply)
{
    QVariantMap response;

    if (replyIsOk(reply, response) < 0)
        return;

    int result_code = response[_("result_code")].toInt();
    if (result_code == 0) {
        passportUamtk();
    } else {
        w->uamNotLogined();
        keepLoginTimer->stop();
        LoginConf &lconf = LoginConf::instance();
        if (lconf.isSweepLogin) {
            // 显示扫码登录入口
            //loginDialog->selectQrCodeTab();
            w->loginDialog->showQrCodeTab();
        } else {
            // 隐藏扫码登录入口
            w->loginDialog->hideQrCodeTab();
        }
    }
}

// 统一认证登录
void NetHelper::loginForUam(ReqParam &param)
{
    ReqParam data;
    QUrl url = QStringLiteral(PASSPORT_LOGIN);
    UserData *ud = UserData::instance();
    QString encode_passwd;
    QList<std::pair<QString, QString>> headers;

    headers.push_back(std::pair("isPasswordCopy", _("")));
    headers.push_back(std::pair("appFlag", _("")));
    if (param.isEmpty()) {
        data.put(_("username"), ud->getUserLoginInfo().userName.toUtf8().toPercentEncoding());
        encode_passwd = sm4_encrypt_ecb(ud->getUserLoginInfo().passwd, SM4_KEY_SECRET);
        data.put(_("password"), '@' + encode_passwd.toUtf8().toPercentEncoding());
        data.put(_("appid"), _(PASSPORT_APPID));
        data.put(_("answer"), _(""));
        post(url, data, &NetHelper::loginForUamReply, headers);
    } else {
        post(url, param, &NetHelper::loginForUamReply, headers);
    }
}

void NetHelper::loginForUamReply(QNetworkReply *reply)
{
    QVariantMap response;

    if (replyIsOk(reply, response) < 0)
        return;

    QString message = QStringLiteral("您的密码很久没有修改了，为降低安全风险，请您<a href=") + BASEURL + PUBLICNAME +
                      QStringLiteral("/forgetPassword/initforgetMyPassword") + QStringLiteral("重新设置密码后再登录");
    int result_code = response[_("result_code")].toInt();
    switch (result_code) {
    case 0:
        passportUamtk();
        break;
    case 91:
    case 92:
    case 94:
    case 95:
    case 97:
        break;
    case 101:
        QMessageBox::warning(w->loginDialog, tr("Warning"), message, QMessageBox::Ok);
        break;
    default:
        message = response[_("result_message")].toString();
        QMessageBox::warning(w->loginDialog, tr("Warning"), message, QMessageBox::Ok);
        break;
    }
    if (message.startsWith(_("用户名或密码错误"))) {
        w->loginDialog->hideSmsVerification();
        w->loginDialog->showUserNamePasswd();
    }
}

// 本地登录+验证码
void NetHelper::loginForLocationPasscode(ReqParam &param)
{
    QUrl url = QStringLiteral(LOGINAYSNSUGGEST);
    post(url, param, &NetHelper::loginForLocationPasscodeReply);
}

// 本地登录
void NetHelper::loginForLocation()
{
    ReqParam param;
    QString encode_passwd;
    UserData *ud = UserData::instance();
    param.put(_("loginUserDTO.user_name"), ud->getUserLoginInfo().userName.toUtf8().toPercentEncoding());
    encode_passwd = sm4_encrypt_ecb(ud->getUserLoginInfo().passwd, SM4_KEY_SECRET);
    param.put(_("userDTO.password"), '@' + encode_passwd.toUtf8().toPercentEncoding());
    QUrl url = QStringLiteral(LOGINAYSNSUGGEST);
    post(url, param, &NetHelper::loginForLocationPasscodeReply);
}

void NetHelper::loginForLocationPasscodeReply(QNetworkReply *reply)
{
    QVariantMap response;

    if (replyIsOk(reply, response) < 0)
        return;

    QVariantMap data = response[_("data")].toMap();
    QString loginCheck = response[_("loginCheck")].toString();
    QStringList messages = response[_("messages")].toStringList();
    if (loginCheck == "Y") {
        w->show();
    } else if (!messages.isEmpty()) {
        QMessageBox::warning(w->loginDialog, tr("Warning"), messages[0], QMessageBox::Ok);
    } else {
        QString msg = data[_("message")].toString();
        if (!msg.isEmpty()) {
            QMessageBox::warning(w->loginDialog, tr("Warning"), msg, QMessageBox::Ok);
        }
    }
}

void NetHelper::loginIndex()
{
    QUrl url(QStringLiteral(USERLOGIN));
    get(url, &NetHelper::ignoreReply);
}

void NetHelper::keepLogin()
{
    initMy12306Api();
}

void NetHelper::loginSuccess()
{
    //w->showStatusBarMessage(_("当前用户：%1").arg(UserData::instance()->getUserLoginInfo().account));
    w->uamLogined();
    initMy12306Api();
    if (!keepLoginTimer->isActive()) {
        keepLoginTimer->start();
    }
    getPassengerInfo();
    getMyPublicIp();
    leftTicketInit();
#ifdef HAS_CDN
    UserData *ud = UserData::instance();
    if (ud->generalSetting.cdnEnable) {
        NetHelper::instance()->getCdn();
    }
#endif
}

void NetHelper::logout()
{
    QUrl url(_(LOGOUT));
    get(url, &NetHelper::logoutReply);
}

void NetHelper::logoutReply(QNetworkReply *reply)
{
    if (checkReplyOk(reply) < 0)
        return;
    w->logoutSuccess();
}

void NetHelper::createQrCode()
{
    ReqParam data;
    QUrl url = QStringLiteral(QR64);
#ifdef HAS_CDN
    QString h = cdn.getRandomCdn();
    if (!h.isEmpty()) {
        cdn.setMainCdn(h);
    }
#endif
    data.put(_("appid"), _(PASSPORT_APPID));
    post(url, data, &NetHelper::createQrCodeReply);
    w->loginDialog->showLoadingQrCode();
}

void NetHelper::createQrCodeReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        enum QNetworkReply::NetworkError errorNo = reply->error();

        if (errorNo != QNetworkReply::NoError) {
            if (errorNo != QNetworkReply::HostNotFoundError &&
                errorNo != QNetworkReply::TimeoutError) {
                createQrCode();
            }
            return;
        }
        return;
    }
    int result_code = varMap[_("result_code")].toInt();
    if (result_code != 0) {
        // Has error
        return;
    }

    w->loginDialog->showLoadedQrCode(varMap);
}

void NetHelper::checkQrCode(const QString &uuid)
{
    QUrl url = QStringLiteral(CHECKQR);
    ReqParam param;
    param.put(_("RAIL_DEVICEID"), _(""));
    param.put(("RAIL_EXPIRATION"), _(""));
    param.put(_("uuid"), uuid);
    param.put(_("appid"), _(PASSPORT_APPID));
    post(url, param, &NetHelper::checkQrCodeReply);
}

void NetHelper::checkQrCodeReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0)
        return;

    int result_code = varMap[_("result_code")].toInt();
    w->loginDialog->updateQrCodeStatus(result_code);
}

void NetHelper::sendSmsRequest(const QString &idCardNumTail)
{
    ReqParam param;
    UserData *ud = UserData::instance();
    param.put(_("appid"), _(PASSPORT_APPID));
    param.put(_("username"), ud->getUserLoginInfo().userName);
    param.put(_("castNum"), idCardNumTail);
    QUrl url = QStringLiteral(SMS_VERIFICATION);
    post(url, param, &NetHelper::sendSmsRequestReply);
}

void NetHelper::sendSmsRequestReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0)
        return;
    QString message;
    int result_code = varMap[_("result_code")].toInt();
    switch (result_code) {
    case 0:
    case 6:
    case 11:
        message = varMap[_("result_message")].toString();
    default:
        break;
    }
    if (!message.isEmpty()) {
        w->loginDialog->remindLabel2->setText(message);
    }
}

void NetHelper::queryTicket()
{
    QUrl url;
    UserConfig &uc = UserData::instance()->getUserConfig();
    QString args;

    args = _("?leftTicketDTO.train_date=%1&leftTicketDTO.from_station=%2&leftTicketDTO.to_station=%3&purpose_codes=ADULT")
               .arg(uc.tourDate, uc.staFromCode, uc.staToCode);
    url.setUrl(queryLeftTicketUrl + args);

    QList<std::pair<QString, QString>> headers;
    headers.append(std::pair<QString, QString>("If-Modified-Since", "0"));
    headers.append(std::pair<QString, QString>("Cache-Control", "no-cache"));
    get2(url, &NetHelper::queryTicketReply, headers);
}

void NetHelper::queryTicketReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    //qDebug() << varMap;

    bool status = varMap[QStringLiteral("status")].toBool();
    if (!status) {
        QStringList messages = varMap[QStringLiteral("messages")].toStringList();
        if (!messages.isEmpty()) {
            w->formatOutput(_("查询失败，原因: %1").arg(messages[0]));
        }
        QString queryUri = varMap[_("c_url")].toString();
        int idx = queryUri.indexOf('?');
        if (idx != -1) {
            QString s = queryUri.first(idx);
            queryLeftTicketUrl = _(BASEURL PUBLICNAME) + '/' + s;
        }
        return;
    }
    QVariantMap data = varMap[QStringLiteral("data")].toMap();
    w->processQueryTicketReply(data);
}

void NetHelper::queryDiffDateTicket(const QString &date)
{
    QUrl url;
    UserConfig &uc = UserData::instance()->getUserConfig();
    url.setUrl(_(BASEURL PUBLICNAME "/leftTicket/queryE?leftTicketDTO.train_date=%1"
                                    "&leftTicketDTO.from_station=%2&leftTicketDTO.to_station=%3&purpose_codes=ADULT")
                   .arg(date, uc.staFromCode, uc.staToCode));

    QList<std::pair<QString, QString>> headers;
    headers.append(std::pair<QString, QString>("If-Modified-Since", "0"));
    headers.append(std::pair<QString, QString>("Cache-Control", "no-cache"));
    get2(url, &NetHelper::queryDiffDateTicketReply, headers);
}

void NetHelper::queryDiffDateTicketReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handlecandidateError();
        return;
    }
    //qDebug() << varMap;
    UserData *ud = UserData::instance();
    QVariantMap data = varMap[QStringLiteral("data")].toMap();
    bool status = varMap[QStringLiteral("status")].toBool();
    if (!status) {
        ud->candidateRunStatus = EGETDIFFDATEDATAFAILED;
        handlecandidateError();
        return;
    }
    QNetworkRequest req = reply->request();
    QUrl url = req.url();
    QString date;
    for (auto &d : ud->candidateInfo.diffDateTrain) {
        if (url.url().contains(d.date)) {
            d.hasUpdate = true;
            date = d.date;
            break;
        }
    }
    if (date.isEmpty()) {
        handlecandidateError();
        return;
    }

    QVariantMap stationMap = data[_("map")].toMap();

    if (stationMap.isEmpty()) {
        ud->runStatus = EGETDIFFDATEDATAFAILED;
        handlecandidateError();
        return;
    }
    QVariantList resultList = data[_("result")].toList();
    if (resultList.isEmpty()) {
        ud->runStatus = EGETDIFFDATEDATAFAILED;
        handlecandidateError();
        return;
    }
    QVector<QStringList> allTrain;
    for (auto &d : resultList) {
        QString train = d.toString();
        QStringList trainInfo = train.split('|');

        if (trainInfo.size() < ESEATTYPES) {
            continue;
        }
        if (trainInfo[ESTATIONTRAINCODE].isEmpty()) {
            continue;
        }
        if (trainInfo[ESECRETSTR].isEmpty()) {
            continue;
        }
        if (ud->candidateSetting.isCandidate) {
            allTrain.push_back(trainInfo);
        }
    }

    if (!allTrain.isEmpty()) {
        Analysis ana(allTrain);
        ana.mayCandidate(stationMap, date);
    } else {
        handlecandidateError();
    }
}

void NetHelper::queryTrainStopStation(const QList<QString> &args)
{
    QUrl url;
    QString argsStr;

    if (args.size() < 4)
        return;
    argsStr = _("?train_no=%1&from_station_telecode=%2&to_station_telecode=%3&depart_date=%4")
               .arg(args[0], args[1], args[2], args[3]);
    url.setUrl(_(QEURYTRAINSTOPSTATION) + argsStr);
    get(url, &NetHelper::queryTrainStopStationReply);
}

void NetHelper::queryTrainStopStationReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0)
        return;
    //qDebug() << varMap;
    w->processStopStationReply(varMap);
}

void NetHelper::passportUamtk()
{
    QUrl url(QStringLiteral(PASSPORT_UAMTK));
    ReqParam param;
    param.put(_("appid"), _(PASSPORT_APPID));

    post(url, param, &NetHelper::passportUamtkReply);
}

void NetHelper::passportUamtkReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0)
        return;
    //qDebug() << varMap;
    QString message = varMap[_("result_message")].toString();
    if (varMap[_("result_code")].toInt() == 0) {
        QString apptk = varMap[_("newapptk")].toString();
        if (!apptk.isEmpty())
            UserData::instance()->setApptk(apptk);
        passportUamtkClient(apptk);
    }  else if (!message.isEmpty()) {
        QMessageBox::warning(w->loginDialog, tr("Warning"), message, QMessageBox::Ok);
    }
}

void NetHelper::passportUamtkClient(const QString &apptk)
{
    QUrl url(_(UAMAUTHCLIENT));
    ReqParam param;
    param.put(_("tk"), apptk);

    post(url, param, &NetHelper::passportUamtkClientReply);
}

void NetHelper::passportUamtkClientReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0)
        return;
    //qDebug() << varMap;
    QString message = varMap[_("result_message")].toString();
    if (varMap[_("result_code")].toInt() == 0) {
        QString account = varMap[_("username")].toString();
        if (!account.isEmpty())
            UserData::instance()->getUserLoginInfo().account = account;
        loginSuccess();
    }  else if (!message.isEmpty()) {
        QMessageBox::warning(w->loginDialog, tr("Warning"), message, QMessageBox::Ok);
    }
}

void NetHelper::getPassengerInfo()
{
    QUrl url(QStringLiteral(GETPASSENGERINFODTOS));
    ReqParam param;
    param.put("_json_attr", "");

    post(url, param, &NetHelper::getPassengerInfoReply);
}

void NetHelper::getPassengerInfoReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0)
        return;
    //qDebug() << varMap;
    bool status = varMap[QStringLiteral("status")].toBool();
    if (!status) {
        w->formatOutput(_("获取乘车人失败，status为false"));
        return;
    }
    QVariantMap data = varMap[QStringLiteral("data")].toMap();
    if (data.isEmpty()) {
        w->formatOutput(_("获取乘车人失败，数据为空"));
        return;
    }
    bool isExist = data[QStringLiteral("isExist")].toBool();
    if (!isExist) {
        QString exMsg = data[_("exMsg")].toString();
        if (!exMsg.isEmpty()) {
            w->formatOutput(_("获取乘车人失败，原因：%1").arg(exMsg));
        }
        bool noLogin = data[_("noLogin")].toBool();
        if (noLogin) {
            w->passengerDialog->hide();
            w->uamNotLogined();
        }
        return;
    }
    QVariantList list = data[QStringLiteral("normal_passengers")].toList();
    if (list.isEmpty()) {
        w->formatOutput(_("乘车人列表为空"));
        return;
    }
    //qDebug() << list;

    UserData *ud = UserData::instance();
    ud->passenger.clear();
    w->passengerDialog->clearPassenger();

    foreach (auto i, list) {
        QVariantMap map = i.toMap();
        struct PassengerInfo pinfo;
        pinfo = ud->setPassengerInfo(map);
        ud->passenger.push_back(pinfo);
        w->passengerDialog->addUnSelectedPassenger(pinfo.passName);
    }
    list = data[_("dj_passengers")].toList();
    if (!list.isEmpty()) {
        foreach (auto i, list) {
            QVariantMap map = i.toMap();
            struct PassengerInfo pinfo;
            pinfo = ud->setPassengerInfo(map);
            ud->djPassenger.push_back(pinfo);
            w->passengerDialog->addUnSelectedPassenger(pinfo.passName);
        }
    }
    QString message = data[QStringLiteral("notify_for_gat")].toString();
    if (!message.isEmpty())
        QMessageBox::information(w->loginDialog, tr("温馨提示"), message, QMessageBox::Ok);
}

void NetHelper::checkUser()
{
    QUrl url(QStringLiteral(LOGINCHECKUSER));
    ReqParam param;
    param.put(_("_json_att"), _(""));

    QList<std::pair<QString, QString>> headers;
    headers.append(std::pair<QString, QString>("If-Modified-Since", "0"));
    headers.append(std::pair<QString, QString>("Cache-Control", "no-cache"));
    post(url, param, &NetHelper::checkUserReply, headers);
    w->formatOutput(_("正在校验登陆状态..."));
    qDebug() << __FUNCTION__;
}

void NetHelper::checkUserReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    qDebug() << __FUNCTION__;
    if (replyIsOk(reply, varMap) < 0) {
        handleError();
        QByteArray b = varMap[_("data")].toByteArray();
        if (!b.isEmpty()) {
            QString s = QString::fromUtf8(b);
            if (s.contains(_("/otn/login/init"))) {
                w->formatOutput(_("当前登陆状态已失效，需要重新登陆"));
                w->startOrStopGrabTicket();
                getLoginConf();
            }
        }
        return;
    }

    qDebug() << varMap;
    QVariantMap data = varMap[QStringLiteral("data")].toMap();
    if (data.isEmpty()) {
        handleError();
        qDebug() << "data is empty";
        return;
    }
    bool flag = data[QStringLiteral("flag")].toBool();
    if (!flag) {
        handleError();
        getLoginConf();
        w->formatOutput(_("当前登陆状态已失效，需要重新登陆"));
        w->startOrStopGrabTicket();
        qDebug() << "flag is false";
        return;
    }

    submitOrderRequest();
}

void NetHelper::initDc()
{
    QUrl url = QStringLiteral(INITDC);
    ReqParam param;
    post(url, param, &NetHelper::initDcReply);
    qDebug() << __FUNCTION__;
}

void NetHelper::initDcReply(QNetworkReply *reply)
{
    if (checkReplyOk(reply) < 0) {
        handleError();
        return;
    }
    QString text = QString::fromUtf8(reply->readAll());
    //qDebug() << text;
    UserData *ud = UserData::instance();
    QString tokenRemaind = _("globalRepeatSubmitToken = '");
    QString ticketInfoForPassengerForm = _("ticketInfoForPassengerForm");
    int beginPos = text.indexOf(ticketInfoForPassengerForm);
    QStringList tokenList = {
        _("'purpose_codes':'"),
        _("'train_location':'"),
        _("'isAsync':'"),
        _("'key_check_isChange':'"),
        _("'leftTicketStr':'")
    };
    QString result;

    int tokenPos = text.indexOf(tokenRemaind);
    if (tokenPos != -1) {
        int tokenEndPos = text.indexOf('\'', tokenPos + tokenRemaind.length());
        if (tokenEndPos != -1) {
            ud->submitTicketInfo.repeatSubmitToken = text.sliced(tokenPos + tokenRemaind.length(),
                                                                 tokenEndPos - (tokenPos + tokenRemaind.length()));
            qDebug() << ud->submitTicketInfo.repeatSubmitToken;
        }
    }

    ud->submitTicketInfo.purposeCodes.clear();
    ud->submitTicketInfo.trainLocation.clear();
    ud->submitTicketInfo.keyCheckIsChange.clear();
    ud->submitTicketInfo.leftTicketStr.clear();

    for (auto &token : tokenList) {
        tokenPos = text.indexOf(token, beginPos);
        if (tokenPos != -1) {
            int tokenEndPos = text.indexOf('\'', tokenPos + token.length());
            if (tokenEndPos != -1) {
                result = text.sliced(tokenPos + token.length(),
                                          tokenEndPos - (tokenPos + token.length()));
                if (token == _("'purpose_codes':'")) {
                    ud->submitTicketInfo.purposeCodes = result;
                } else if (token == _("'train_location':'")) {
                    ud->submitTicketInfo.trainLocation = result;
                } else if (token == _("'isAsync':'")) {
                    ud->submitTicketInfo.isAsync = result == "1";
                } else if (token == _("'key_check_isChange':'")) {
                    ud->submitTicketInfo.keyCheckIsChange = result;
                } else if (token == _("'leftTicketStr':'")) {
                    ud->submitTicketInfo.leftTicketStr = result;
                }
                //qDebug() << token << result;
            }
        }
    }

    if (ud->submitTicketInfo.repeatSubmitToken.isEmpty() ||
        ud->submitTicketInfo.purposeCodes.isEmpty() ||
        ud->submitTicketInfo.trainLocation.isEmpty() ||
        ud->submitTicketInfo.leftTicketStr.isEmpty()) {
        w->formatOutput(_("下单失败，无法从服务器获取指定的参数！"));
        handleError();
        return;
    }
    checkOrderInfo();
}

void NetHelper::initMy12306Api()
{
    QUrl url = QStringLiteral(INITMY12306API);
    ReqParam param;
    post(url, param, &NetHelper::initMy12306ApiReply);
    qDebug() << __FUNCTION__;
}

void NetHelper::initMy12306ApiReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        QByteArray b = varMap[_("data")].toByteArray();
        if (!b.isEmpty()) {
            QString s = QString::fromUtf8(b);
            if (s.contains(_("/otn/login/init"))) {
                UserData *ud = UserData::instance();
                w->formatOutput(_("当前登陆状态已失效，需要重新登陆"));
                if (ud->runStatus != EIDLE) {
                    if (ud->runStatus == ESUBMITORDER) {
                        handleError();
                    }
                    w->startOrStopGrabTicket();
                }
                getLoginConf();
            }
        }
        return;
    }
    //qDebug() << varMap;
#if 0
    bool status = varMap[QStringLiteral("status")].toBool();
    if (!status) {
        return;
    }
    QVariantMap data = varMap[QStringLiteral("data")].toMap();
    if (data.isEmpty()) {
        return;
    }
#endif
}

void NetHelper::handleError()
{
    UserData::instance()->recoverRunStatus();
    netStatInc(ESUBMITFAILED);
}

void NetHelper::submitOrderRequest()
{
    QUrl url(QStringLiteral(SUBMITORDERREQUEST));
    ReqParam param;
    UserData *ud = UserData::instance();
    param.put(_("secretStr"), ud->submitTicketInfo.secretStr);
    param.put(_("train_date"), ud->submitTicketInfo.date);
    param.put(_("back_train_date"), ud->submitTicketInfo.date);
    param.put(_("tour_flag"), _("dc"));
    param.put(_("purpose_codes"), _("ADULT"));
    param.put(_("query_from_station_name"), ud->submitTicketInfo.fromStationName);
    param.put(_("query_to_station_name"), ud->submitTicketInfo.toStationName);
    param.put(_("bed_level_info"), ud->submitTicketInfo.bedLevelInfo);
    param.put(_("seat_discount_info"), ud->submitTicketInfo.seatDiscountInfo);
    param.put(_("undefined"), _(""));
    param.finish();

    //UserData::instance()->submitTicketInfo.repeatSubmitToken = _("");

    post(url, param, &NetHelper::submitOrderRequestReply);
    w->formatOutput(_("正在提交订单..."));
    //ud->setRunStatus(ESUBMITORDER);
    qDebug() << __FUNCTION__;
    netStatInc(ESUBMIT);
}

void NetHelper::submitOrderRequestReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    qDebug() << __FUNCTION__;
    if (replyIsOk(reply, varMap) < 0) {
        handleError();
        return;
    }
    //qDebug() << varMap;
    bool status = varMap[QStringLiteral("status")].toBool();
    if (!status) {
        QStringList messages = varMap[QStringLiteral("messages")].toStringList();
        if (!messages.isEmpty()) {
            if (messages[0].contains(_("\"../view/train_order.html\""))) {
                messages[0].replace(_(".."), _("https://kyfw.12306.cn/otn"));
            }
            w->formatOutput(QStringLiteral("提交订单失败! 错误描述：%1").arg(messages[0]));
        }
        handleError();
        return;
    }
    initDc();
}

void NetHelper::checkOrderInfo()
{
    QUrl url(QStringLiteral(CHECKORDERINFO));
    ReqParam param;
    UserData *ud = UserData::instance();
    param.put(_("cancel_flag"), _("2"));
    param.put(_("bed_level_order_num"), _("000000000000000000000000000000"));
    param.put(_("passengerTicketStr"), ud->submitTicketInfo.passengerTicketInfo);
    param.put(_("oldPassengerStr"), ud->submitTicketInfo.oldPassengerTicketInfo);
    param.put(_("tour_flag"), _("dc"));
    param.put(_("whatsSelect"), ud->whatsSelect(true) ? "1" : "0");
    param.put(_("sessionId"), _(""));
    param.put(_("sig"), _(""));
    param.put(_("scene"), _("nc_login"));
    param.put(_("_json_att"), _(""));
    param.put(_("REPEAT_SUBMIT_TOKEN"), ud->submitTicketInfo.repeatSubmitToken);

    post(url, param, &NetHelper::checkOrderInfoReply);
    w->formatOutput(_("正在校验订单..."));
    //qDebug() << param.get();
    qDebug() << __FUNCTION__;
}

void NetHelper::checkOrderInfoReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handleError();
        return;
    }
    //qDebug() << varMap;
    bool status = varMap[QStringLiteral("status")].toBool();
    if (!status) {
        QStringList messages = varMap[QStringLiteral("messages")].toStringList();
        if (!messages.isEmpty()) {
            w->formatOutput(QStringLiteral("出票失败! 错误描述：%1").arg(messages[0]));
        }
        handleError();
        return;
    }
    QVariantMap data = varMap[QStringLiteral("data")].toMap();
    bool submitStatus = data[QStringLiteral("submitStatus")].toBool();
    if (!submitStatus) {
        bool isRelogin = data[QStringLiteral("isRelogin")].toBool();
        if (!isRelogin) {
            bool isNoActive = data[QStringLiteral("isNoActive")].toBool();
            QString errMsg = data[QStringLiteral("errMsg")].toString();
            if (isNoActive) {
                w->formatOutput(errMsg);
            } else {
                bool checkSeatNum = data[QStringLiteral("checkSeatNum")].toBool();
                if (checkSeatNum) {
                    w->formatOutput(QString(QStringLiteral("很抱歉，无法提交您的订单! 原因： %1")).arg(errMsg));
                } else {
                    w->formatOutput(QString(QStringLiteral("出票失败! 原因： %1")).arg(errMsg));
                }
            }
        } else {
            // redirect to BASEURL PUBLICNAME + "view/index.html?random=" + new Date().getTime()
            w->formatOutput(_("当前登陆状态已失效，需要重新登陆"));
            w->startOrStopGrabTicket();
            getLoginConf();
        }
        handleError();
        return;
    }
    w->formatOutput(_("订单校验完成"));
    //int intervalTime = data[QStringLiteral("ifShowPassCodeTime")].toInt();
    canChooseSeats = data[QStringLiteral("canChooseSeats")].toString() == "Y";
    chooseSeat = data[QStringLiteral("choose_Seats")].toString();
    //bool canChooseBeds = data[QStringLiteral("canChooseBeds")].toString() == "Y";
    //bool isCanChooseMid = data[QStringLiteral("isCanChooseMid")].toString() == "Y";
    UserData *ud = UserData::instance();
    if (ud->submitTicketInfo.isAsync) {
        getQueueCount();
    } else {
        confirmSingle();
    }
}

void NetHelper::getQueueCount()
{
    QUrl url(QStringLiteral(GETQUEUECOUNT));
    ReqParam param;
    UserData *ud = UserData::instance();
    // "ddd MMM dd yyyy 00:00:00 GMT+0800 (中国标准时间)"
    QString dateStr = QDate::fromString(ud->submitTicketInfo.date, _("yyyy-MM-dd"))
                          .toString(_("ddd+MMM+dd+yyyy"));
    dateStr.append(_("+00%3A00%3A00+GMT%2B0800+(%E4%B8%AD%E5%9B%BD%E6%A0%87%E5%87%86%E6%97%B6%E9%97%B4)"));
    param.put(_("train_date"), dateStr);
    param.put(_("train_no"), ud->submitTicketInfo.trainNo);
    param.put(_("stationTrainCode"), ud->submitTicketInfo.trainCode);
    param.put(_("seatType"), !ud->submitTicketInfo.submitSeatType.isEmpty() ?
                                 ud->submitTicketInfo.submitSeatType[0].second : _(""));
    param.put(_("fromStationTelecode"), ud->submitTicketInfo.fromStationCode);
    param.put(_("toStationTelecode"), ud->submitTicketInfo.toStationCode);
    param.put(_("leftTicket"), ud->submitTicketInfo.ypDetailInfo);
    param.put(_("purpose_codes"), ud->submitTicketInfo.purposeCodes);
    param.put(_("train_location"), ud->submitTicketInfo.trainLocation);
    param.put(_("_json_att"), _(""));
    param.put(_("REPEAT_SUBMIT_TOKEN"), ud->submitTicketInfo.repeatSubmitToken);

    post(url, param, &NetHelper::getQueueCountReply);
    w->formatOutput(_("正在获取实时余票及排队信息..."));
    qDebug() << __FUNCTION__;
}

void NetHelper::getQueueCountReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handleError();
        return;
    }
    //qDebug() << varMap;
    bool status = varMap[QStringLiteral("status")].toBool();
    if (!status) {
        QStringList messages = varMap[QStringLiteral("messages")].toStringList();
        if (!messages.isEmpty()) {
            w->formatOutput(QStringLiteral("出票失败! 错误描述：%1").arg(messages[0]));
        }
        handleError();
        return;
    }
    QVariantMap data = varMap[QStringLiteral("data")].toMap();
    bool isRelogin = data[QStringLiteral("isRelogin")].toBool();
    if (isRelogin) {
        w->hide();
        w->loginDialog->show();
        UserData::instance()->setRunStatus(EIDLE);
        netStatInc(ESUBMITFAILED);
        return;
    }
    QStringList ticketList = data[_("ticket")].toString().split(',');
    QString disp;
    bool ok;
    UserData *ud = UserData::instance();
    QChar c = !ud->submitTicketInfo.submitSeatType.isEmpty() ?
                  ud->submitTicketInfo.submitSeatType[0].second : QChar('0');
    int totalRemain = 0;
    int remain = ticketList[0].toInt(&ok);
    if (ok) {
        disp.append(_("本次列车%1余票%2张").arg(seatTypeSubmtiCodeTransToDesc(c))
                        .arg(remain));
        totalRemain = remain;
    } else {
        disp.append(_("本次列车%1余票%2").arg(seatTypeSubmtiCodeTransToDesc(c),
                        ticketList[0]));
    }
    if (ticketList.length() > 1) {
        remain = ticketList[1].toInt(&ok);
        if (ok) {
            disp.append(_(", 无座余票%1张").arg(remain));
            totalRemain += remain;
            if (totalRemain <= 0) {
                mayFrozenCurrentTrain(_("实时余票为%1张，该车次余票不足，把%2加入冻结列表%3秒")
                                          .arg(totalRemain)
                                          .arg(ud->submitTicketInfo.trainCode)
                                          .arg(ud->grabSetting.frozenSeconds));
            }
        } else {
            disp.append(_(", 无座余票%1").arg(ticketList[1]));
        }
    }
    if (!disp.isEmpty()) {
        w->formatOutput(disp);
    }
    QString op_2 = data[_("op_2")].toString();
    if (op_2 == "true") {
        w->formatOutput(_("目前排队人数已经超过余票张数, 无法下单！"));
        mayFrozenCurrentTrain(_("无法提交订单，该车次余票不足，把%1加入冻结列表%2秒")
                                  .arg(ud->submitTicketInfo.trainCode)
                                  .arg(ud->grabSetting.frozenSeconds));
        handleError();
        return;
    }
    int countT = data[_("countT")].toInt();
    if (countT > 0) {
        w->formatOutput(_("目前排队人数%1人").arg(countT));
    }
    confirmSingleForQueue();
}

void NetHelper::confirmSingle()
{
    QUrl url(QStringLiteral(CONFIRMSINGLEFORQUEUE));
    ReqParam param;
    UserData *ud = UserData::instance();

    param.put(_("passengerTicketStr"), ud->submitTicketInfo.passengerTicketInfo);
    param.put(_("oldPassengerStr"), ud->submitTicketInfo.oldPassengerTicketInfo);
    param.put(_("purpose_codes"), ud->submitTicketInfo.purposeCodes);
    param.put(_("key_check_isChange"), ud->submitTicketInfo.keyCheckIsChange);
    param.put(_("leftTicketStr"), ud->submitTicketInfo.leftTicketStr.toUtf8().toPercentEncoding());
    param.put(_("train_location"), ud->submitTicketInfo.trainLocation);
    if (canChooseSeats && !chooseSeat.isEmpty() &&
        !ud->submitTicketInfo.submitSeatType.isEmpty()) {
        QChar isSame = ud->submitTicketInfo.submitSeatType[0].second;
        int i;
        for (i = 1; i < ud->submitTicketInfo.submitSeatType.size(); i++) {
            if (ud->submitTicketInfo.submitSeatType[i].second != isSame) {
                break;
            }
        }
        QString selectedSeats;
        if (i == ud->submitTicketInfo.submitSeatType.size() && chooseSeat.contains(isSame)) {
            selectedSeats = w->seatDialog->getChoosedSeats(isSame);
            if (selectedSeats.size() > ud->submitTicketInfo.submitSeatType.size() * 2) {
                selectedSeats.truncate(ud->submitTicketInfo.submitSeatType.size() * 2);
                w->formatOutput(_("提交选座%1，实际座位号以系统出票结果为准").arg(selectedSeats));
            } else {
                if (selectedSeats.size() < ud->submitTicketInfo.submitSeatType.size() * 2) {
                    if (selectedSeats.isEmpty()) {
                        w->formatOutput(_("本次席别未选座，将由系统随机分配座位"));
                    } else {
                        w->formatOutput(_("已选座人数少于提交人数，本次提交不选座，将由系统随机分配座位"));
                        selectedSeats.clear();
                    }
                } else {
                    w->formatOutput(_("提交选座%1，实际座位号以系统出票结果为准").arg(selectedSeats));
                }
            }
        } else {
            w->formatOutput(_("本次提交暂不支持选座，将由系统随机分配座位"));
        }
        param.put(_("choose_seats"), selectedSeats);
    } else {
        param.put(_("choose_seats"), _(""));
        w->formatOutput(_("本次列车暂不支持选座，将由系统自动分配座位"));
    }
    param.put(_("seatDetailType"), _("000"));
    param.put(_("is_jy"), _("N"));
    param.put(_("is_cj"), _("N"));
    param.put(_("whatsSelect"), ud->whatsSelect(true) ? _("1") : _("0"));
    param.put(_("roomType"), _("00"));
    param.put(_("dwAll"), _("N"));
    param.put(_("_json_att"), _(""));
    param.put(_("REPEAT_SUBMIT_TOKEN"), ud->submitTicketInfo.repeatSubmitToken);

    post(url, param, &NetHelper::confirmSingleReply);
    w->formatOutput(_("正在确认订单..."));
    qDebug() << __FUNCTION__;
}

void NetHelper::confirmSingleReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    UserData *ud = UserData::instance();
    if (replyIsOk(reply, varMap) < 0) {
        handleError();
        return;
    }
    //qDebug() << varMap;
    bool status = varMap[_("status")].toBool();
    if (!status) {
        w->formatOutput(_("出票失败! 无法预订本次车票"));
        mayFrozenCurrentTrain(_("无法提交订单，未知错误，把%1加入冻结列表%2秒")
                                  .arg(ud->submitTicketInfo.trainCode)
                                  .arg(ud->grabSetting.frozenSeconds));
        handleError();
        return;
    }
    QVariantMap data = varMap[_("data")].toMap();
    bool submitStatus = data[_("submitStatus")].toBool();
    if (!submitStatus) {
        QString errMsg = varMap[QStringLiteral("errMsg")].toString();
        w->formatOutput(QStringLiteral("出票失败! 原因：%1").arg(errMsg));
        if (errMsg.startsWith(_("对不起，由于您取消次数过多，今日将不能继续受理您的订票请求"))) {
            w->startOrStopGrabTicket();
        }
        mayFrozenCurrentTrain(_("无法提交订单，把%1加入冻结列表%@秒")
                                  .arg(ud->submitTicketInfo.trainCode)
                                  .arg(ud->grabSetting.frozenSeconds));
        handleError();
        return;
    }
    w->formatOutput(_("出票成功! 为避免订单超时取消，请您尽快完成支付"));
    //UserData::instance()->submitTicketInfo.submitSuccess = true;
    grabTicketSuccess();
}

// Async
void NetHelper::confirmSingleForQueue()
{
    QUrl url(QStringLiteral(CONFIRMSINGLEFORQUEUE));
    ReqParam param;
    UserData *ud = UserData::instance();

    param.put(_("passengerTicketStr"), ud->submitTicketInfo.passengerTicketInfo);
    param.put(_("oldPassengerStr"), ud->submitTicketInfo.oldPassengerTicketInfo);
    param.put(_("purpose_codes"), ud->submitTicketInfo.purposeCodes);
    param.put(_("key_check_isChange"), ud->submitTicketInfo.keyCheckIsChange);
    param.put(_("leftTicketStr"), ud->submitTicketInfo.leftTicketStr.toUtf8().toPercentEncoding());
    param.put(_("train_location"), ud->submitTicketInfo.trainLocation);
    if (canChooseSeats && !chooseSeat.isEmpty() &&
        !ud->submitTicketInfo.submitSeatType.isEmpty()) {
        QChar isSame = ud->submitTicketInfo.submitSeatType[0].second;
        int i;
        for (i = 1; i < ud->submitTicketInfo.submitSeatType.size(); i++) {
            if (ud->submitTicketInfo.submitSeatType[i].second != isSame) {
                break;
            }
        }
        QString selectedSeats;
        if (i == ud->submitTicketInfo.submitSeatType.size() && chooseSeat.contains(isSame)) {
            selectedSeats = w->seatDialog->getChoosedSeats(isSame);
            if (selectedSeats.size() > ud->submitTicketInfo.submitSeatType.size() * 2) {
                selectedSeats.truncate(ud->submitTicketInfo.submitSeatType.size() * 2);
                w->formatOutput(_("提交选座%1，实际座位号以系统出票结果为准").arg(selectedSeats));
            } else {
                if (selectedSeats.size() < ud->submitTicketInfo.submitSeatType.size() * 2) {
                    if (selectedSeats.isEmpty()) {
                        w->formatOutput(_("本次席别未选座，将由系统随机分配座位"));
                    } else {
                        w->formatOutput(_("已选座人数少于提交人数，本次提交不选座，将由系统随机分配座位"));
                        selectedSeats.clear();
                    }
                } else {
                    w->formatOutput(_("提交选座%1，实际座位号以系统出票结果为准").arg(selectedSeats));
                }
            }
        } else {
            w->formatOutput(_("本次提交暂不支持选座，将由系统随机分配座位"));
        }
        param.put(_("choose_seats"), selectedSeats);
        qDebug() << selectedSeats;
    } else {
        param.put(_("choose_seats"), _(""));
        w->formatOutput(_("本次列车暂不支持选座，将由系统随机分配座位"));
    }
    param.put(_("seatDetailType"), _("000"));
    param.put(_("is_jy"), _("N"));
    param.put(_("is_cj"), _("Y"));
    param.put(_("encryptedData"), _(""));
    param.put(_("whatsSelect"), ud->whatsSelect(true) ? _("1") : _("0"));
    param.put(_("roomType"), _("00"));
    param.put(_("dwAll"), _("N"));
    param.put(_("_json_att"), _(""));
    param.put(_("REPEAT_SUBMIT_TOKEN"), ud->submitTicketInfo.repeatSubmitToken);

    post(url, param, &NetHelper::confirmSingleForQueueReply);
    w->formatOutput(_("正在确认订单..."));
    qDebug() << param.get();
    qDebug() << __FUNCTION__;
}

void NetHelper::confirmSingleForQueueReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    UserData *ud = UserData::instance();
    if (replyIsOk(reply, varMap) < 0) {
        handleError();
        return;
    }
    qDebug() << varMap;
    bool status = varMap[_("status")].toBool();
    if (!status) {
        w->formatOutput(_("出票失败! 无法预订本次车票"));
        mayFrozenCurrentTrain(_("无法提交订单，未知错误，把%1加入冻结列表%2秒")
                                  .arg(ud->submitTicketInfo.trainCode)
                                  .arg(ud->grabSetting.frozenSeconds));
        handleError();
        return;
    }
    QVariantMap data = varMap[_("data")].toMap();
    bool submitStatus = data[_("submitStatus")].toBool();
    if (!submitStatus) {
        QString errMsg = varMap[QStringLiteral("errMsg")].toString();
        w->formatOutput(QStringLiteral("出票失败! 原因：%1").arg(errMsg));
        mayFrozenCurrentTrain(_("无法提交订单，把%1加入冻结列表%2秒")
                                  .arg(ud->submitTicketInfo.trainCode)
                                  .arg(ud->grabSetting.frozenSeconds));
        handleError();
        return;
    }
    bool isAsync = data[_("isAsync")].toBool();
    if (isAsync) {
        if (!queryOrderTimer) {
            queryOrderTimer = new QTimer;
            queryOrderTimer->setInterval(1000);
            connect(queryOrderTimer, &QTimer::timeout, this, &NetHelper::displayWaitTime);
            queryOrderTimer->start();
            orderWaitTime = 1;
            nextRequestOrderStatusTime = 1;
        }
    } else {
        w->formatOutput(_("出票成功! 为避免订单超时取消，请您尽快完成支付"));
        //UserData::instance()->submitTicketInfo.submitSuccess = true;
        grabTicketSuccess();
    }
}

void NetHelper::displayWaitTime()
{
    if (orderWaitTime == nextRequestOrderStatusTime) {
        queryOderWaitTime();
    }
    QString waitMinStr;
    int minute = orderWaitTime / 60;
    if (minute > 1) {
        waitMinStr.append(_("%1分").arg(minute));
    } else {
        waitMinStr.append(_("1分"));
    }
    if (orderWaitTime > 1)
        --orderWaitTime;
    if (orderWaitTime <= 5) {
        w->formatOutput(_("12306系统正在处理中，请稍等..."));
        if (orderWaitTime <= 0) {
            if (queryOrderTimer) {
                queryOrderTimer->stop();
                delete queryOrderTimer;
                queryOrderTimer = nullptr;
            }
            resultOrderForDcQueue();
        }
    } else if (orderWaitTime <= 30 * 60) {
        w->formatOutput(_("12306系统正在处理中，最新预估等待时间%1，请耐心等待...").arg(waitMinStr));
    } else {
        w->formatOutput(_("12306系统正在处理中，预计等待时间超过30分钟，请耐心等待..."));
    }
}

void NetHelper::queryOderWaitTime()
{
    QUrl url(_(QUERYORDERWAITTIME"?random=%1&tourFlag=dc&_json_att=&REPEAT_SUBMIT_TOKEN=%2")
                 .arg(QDateTime::currentMSecsSinceEpoch()/1000)
                 .arg(UserData::instance()->submitTicketInfo.repeatSubmitToken));
    get(url, &NetHelper::queryOderWaitTimeReply);
    qDebug() << __FUNCTION__;
}

void NetHelper::queryOderWaitTimeReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0)
        return;
    //qDebug() << varMap;
    waitOrderInfo = varMap[_("data")].toMap();
    bool queryOrderWaitTimeStatus = waitOrderInfo[_("queryOrderWaitTimeStatus")].toBool();
    if (!queryOrderWaitTimeStatus) {
        return;
    }

    int waitTime = waitOrderInfo[_("waitTime")].toInt();
    if (waitTime != -100) {
        orderWaitTime = waitTime;
        int d = waitTime / 1.5;
        d = d > 60 ? 60 : d;
        int n = waitTime - d;
        nextRequestOrderStatusTime = n > 0 ? n : 1;
    }
}

void NetHelper::resultOrderForDcQueue()
{
    QUrl url(_(RESULTORDERFORDCQUEUE));
    ReqParam param;
    UserData *ud = UserData::instance();

    if (orderWaitTime == -1 || orderWaitTime == -100) {
        QString orderId = waitOrderInfo[_("orderId")].toString();
        param.put(_("orderSequence_no"), orderId);
        post(url, param, &NetHelper::resultOrderForDcQueueReply);
    } else if (orderWaitTime == -2){
        //int errorCode = waitOrderInfo[_("errorCode")].toInt();
        QString msg = waitOrderInfo[_("msg")].toString();
        w->formatOutput(_("订票失败! 原因：%1").arg(msg));
        if (msg.startsWith(_("对不起，由于您取消次数过多，今日将不能继续受理您的订票请求"))) {
            w->startOrStopGrabTicket();
        }
        mayFrozenCurrentTrain(_("无法提交订单，把%1加入冻结列表%2秒")
                                  .arg(ud->submitTicketInfo.trainCode)
                                  .arg(ud->grabSetting.frozenSeconds));
        handleError();
    } else if (orderWaitTime == -3) {
        w->formatOutput(_("哎呀,订票失败! 订单已撤销"));
        mayFrozenCurrentTrain(_("无法提交订单，把%1加入冻结列表%2秒")
                                  .arg(ud->submitTicketInfo.trainCode)
                                  .arg(ud->grabSetting.frozenSeconds));
        handleError();
    } else {
        // Redirect to BASEURL PUBLICNAME "view/train_order.html?type=1&random=" + new Date().getTime()
    }
}

void NetHelper::resultOrderForDcQueueReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0)
        return;
    qDebug() << varMap;
    bool status = varMap[_("status")].toBool();
    if (!status) {
        w->formatOutput(_("出票成功"));
        grabTicketSuccess();
        return;
    }
    QVariantMap data = varMap[_("data")].toMap();
    bool submitStatus = data[_("submitStatus")].toBool();
    if (!submitStatus) {
        w->formatOutput(QStringLiteral("出票成功"));
        return;
    }
    QString orderId = waitOrderInfo[_("orderId")].toString();
    w->formatOutput(_("出票成功! 订单号：%1, 为避免订单超时取消，请您尽快完成支付")
                        .arg(orderId));
    //UserData::instance()->submitTicketInfo.submitSuccess = true;
    grabTicketSuccess();
}

void NetHelper::getStationNameTxt()
{
    QUrl url(_(STATIONNAMEURL STATIONNAMEVERSION));

    get(url, &NetHelper::getStationNameTxtReply);
}

void NetHelper::getStationNameTxtReply(QNetworkReply *reply)
{
    QByteArray text = reply->readAll();

    if (!text.isEmpty()) {
        w->setStationNameCompleter(text);
        saveStationNameFile(text);
    }
}

void NetHelper::saveStationNameFile(const QByteArray &nameText)
{
    QFile file(QStringLiteral("./station_name_" STATIONNAMEVERSION ".txt"));

    if (file.open(QIODevice::WriteOnly)) {
        file.write(nameText);
        file.close();
        return;
    }
}

void NetHelper::mayFrozenCurrentTrain(const QString &dispMsg)
{
    UserData *ud = UserData::instance();

    if (ud->grabSetting.autoFrozenTrain) {
        w->formatOutput(dispMsg);
        frozenTrain.addFrozenTrain(_("%1 (%2 %3").arg(ud->submitTicketInfo.trainCode,
                                                   ud->submitTicketInfo.fromStationName,
                                                   ud->submitTicketInfo.toStationName),
                                   ud->grabSetting.frozenSeconds);
    }
}

bool NetHelper::queryFrozenTrain(const QString &trainDesc)
{
    return frozenTrain.queryTrainIsFrozen(trainDesc);
}

void NetHelper::grabTicketSuccess()
{
    //UserData::instance()->runStatus = EIDLE;
    // 结束任务
    UserData *ud = UserData::instance();
    ud->lastRunSuccess = true;
    sendMail();
    w->settingDialog->sendWxNotify(_("%1 %2 %3 %4 %5 %6 %7 %8 订单出票成功! 为避免订单超时取消，请您尽快完成支付")
                                       .arg(ud->submitTicketInfo.date, ud->submitTicketInfo.trainCode,
                                            ud->submitTicketInfo.fromStationName, ud->submitTicketInfo.toStationName,
                                            ud->submitTicketInfo.fromTime, ud->submitTicketInfo.toTime,
                                            ud->submitTicketInfo.travelTime,
                                            seatTypeSubmtiCodeTransToDesc(ud->submitTicketInfo.submitSeatType[0].second)));
    w->startOrStopGrabTicket();
    if (ud->generalSetting.playMusic) {
        w->startOrStopPlayMusic();
    }

    // 调用支付
    if (ud->paySetting.activePay) {
        getMyPublicIp();
        payCheckNew();
    }
}

void NetHelper::showCandidateWarn(const QString &face_check_code, bool is_show_qrcode)
{
    if (face_check_code == _("01") ||
        face_check_code == _("11")) {
        w->formatOutput(_("证件信息正在审核中，请您耐心等待，审核通过后可继续完成候补操作"));
    } else if (face_check_code == _("03") ||
               face_check_code == _("13")) {
        w->formatOutput(_("证件信息审核失败，请检查所填写的身份信息内容与原证件是否一致"));
    } else if (face_check_code == _("04") ||
               face_check_code == _("14")) {

        if (!is_show_qrcode) {
            w->formatOutput(_("通过人证一致性核验的用户及激活的“铁路畅行”会员可以提交候补需求，"
                              "请您按照操作说明在铁路12306app上完成人证核验"));
        }
    } else if (face_check_code == _("02") ||
               face_check_code == _("12")) {
        w->formatOutput(_("系统忙，请稍后再试！"));
    }
}

void NetHelper::handlecandidateError()
{
    UserData *ud = UserData::instance();
    ud->candidateInfo.diffDateTrain.clear();
    ud->candidateInfo.submitSecretStr.clear();
    ud->candidateInfo.allSeatType.clear();
    ud->recoverRunStatus();
    netStatInc(ECANDIDATEFAILED);
}

void NetHelper::candidateEntry(const struct CandidateDateInfo &dInfo)
{
    UserData *ud = UserData::instance();

    if (ud->candidateInfo.diffDateTrain.isEmpty()) {
        chechFace(dInfo);
    } else {
        for (auto &d : ud->candidateInfo.diffDateTrain) {
            if (d.date == dInfo.date) {
                d = dInfo;
            } else {
                if (!d.hasUpdate) {
                    return;
                }
            }
        }
        confirmHB();
    }
}

void NetHelper::chechFace(const struct CandidateDateInfo &dInfo)
{
    QUrl url(_(CHECHFACE));
    ReqParam param;
    QString secretList;
    QString tmp;

    // secretStr#席别代码|
    if (!dInfo.train.isEmpty() && !dInfo.train[0].seatType.isEmpty()) {
        tmp.append(dInfo.train[0].secretStr);
        tmp.append('#');
        tmp.append(dInfo.train[0].seatType[0]);
        tmp.append('|');
        secretList.append(tmp.toUtf8().toPercentEncoding());
        tmp.clear();

        UserData *ud = UserData::instance();
        ud->candidateInfo.diffDateTrain.append(dInfo);
        ud->candidateInfo.submitSecretStr = secretList;
        ud->setRunStatus(ESUBMITCANDIDATE);
        param.put(_("secretList"), secretList);
        param.put(_("_json_att"), _(""));
        qDebug() << param.get();
        QList<std::pair<QString, QString>> headers;
        headers.append(std::pair<QString, QString>("If-Modified-Since", "0"));
        headers.append(std::pair<QString, QString>("Cache-Control", "no-cache"));
        post(url, param, &NetHelper::checkFaceReply, headers);
        netStatInc(ECANDIDATE);
    }
}

void NetHelper::checkFaceReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handlecandidateError();
        return;
    }
    qDebug() << varMap;

    bool status = varMap[_("status")].toBool();
    if (!status) {
        QStringList messages = varMap[_("messages")].toStringList();
        QString m = messages.isEmpty() ? _("status is false") : messages[0];
        w->formatOutput(_("候补下单时遇到错误: ") + m);
        qDebug() << m;
        UserData *ud = UserData::instance();
        if (m.startsWith(_("目前您已有待支付的候补订单"))) {
            if (ud->candidateSetting.onlyCandidate) {
                ud->lastRunSuccess = true;
                w->startOrStopGrabTicket();
            } else {
                ud->candidateSetting.isCandidate = false;
                w->settingDialog->closeCandidate();
                ud->setRunStatus(EGRABTICKET);
                ud->candidateRunStatus = EIDLE;
            }
        }
        return;
    }
    QVariantMap data = varMap[_("data")].toMap();
    bool login_flag = data[_("login_flag")].toBool();
    if (login_flag) {
        bool face_flag = data[_("face_flag")].toBool();
        QString face_check_code = data[_("face_check_code")].toString();
        if (face_flag == false) {
            bool is_show_qrcode = data[_("is_show_qrcode")].toBool();
            showCandidateWarn(face_check_code, is_show_qrcode);
            handlecandidateError();
            w->formatOutput(_("候补下单时遇到错误: face flag is false"));
            QStringList messages = varMap[_("messages")].toStringList();
            qDebug() << messages;
        } else {
            submitCandidateOrderRequest();
        }
    } else {
        handlecandidateError();
        qDebug() << "login flag is false";
        w->formatOutput(_("候补下单时遇到错误: login flag is false"));
        QStringList messages = varMap[_("messages")].toStringList();
        qDebug() << messages;
    }
}

void NetHelper::submitCandidateOrderRequest()
{
    QUrl url(_(SUBMITCANDIDATEORDERQUEST));
    ReqParam param;

    param.put(_("secretList"), UserData::instance()->candidateInfo.submitSecretStr);
    param.put(_("_json_att"), _(""));
    post(url, param, &NetHelper::submitCandidateOrderRequestReply);
}

void NetHelper::submitCandidateOrderRequestReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handlecandidateError();
        return;
    }
    qDebug() << varMap;

    QVariantMap data = varMap[_("data")].toMap();
    bool flag = data[_("flag")].toBool();
    if (flag) {
        passengerInitApi();
    } else {
        QString face_check_code = data[_("face_check_code")].toString();
        bool is_show_qrcode = data[_("is_show_qrcode")].toBool();
        showCandidateWarn(face_check_code, is_show_qrcode);
        handlecandidateError();
    }
}

void NetHelper::passengerInitApi()
{
    QUrl url(_(PASSENGERINITAPI));
    ReqParam param;

    post(url, param, &NetHelper::passengerInitApiReply);
}

void NetHelper::passengerInitApiReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handlecandidateError();
        return;
    }
    qDebug() << varMap;

    QVariantMap data = varMap[_("data")].toMap();
    QList<QVariant> jzdhDiffSelect = data[_("jzdhDiffSelect")].toList();
    int endTime;
    if (jzdhDiffSelect.isEmpty()) {
        w->formatOutput(_("服务器没有返回候补截止时间！"));
        handlecandidateError();
        return;
    }
    int endCandidateMinutes = UserData::instance()->candidateSetting.selectedEndCandidateMinutes;
    QVector<int> t(jzdhDiffSelect.size(), 0);
    QVector<QString> m;
    QString disp = _("可选的候补截止时间为：");
    QString dispTime;
    QString tmp;
    int i;
    for (i = 0; i < jzdhDiffSelect.size(); i++) {
        endTime = jzdhDiffSelect[i].toInt();
        if (endTime >= 1440) {
            tmp = _("%1天,").arg(endTime / 1440);
            dispTime.append(tmp);
            m.append(tmp);
        } else if (endTime >= 60) {
            tmp = _("%1小时,").arg(endTime / 60);
            dispTime.append(tmp);
            m.append(tmp);
        } else {
            tmp = _("%1分钟,").arg(endTime);
            dispTime.append(tmp);
            m.append(tmp);
        }
        t[i] = qAbs(endTime - endCandidateMinutes);
    }
    if (!dispTime.isEmpty()) {
        dispTime.truncate(dispTime.length() - 1);
        w->formatOutput(disp + dispTime);
    }
    int min = INT_MAX;
    int s = -1;
    for (i = 0; i < t.size(); i++) {
        if (t[i] < min) {
            min = t[i];
            s = i;
        }
    }
    if (s != -1) {
        m[s].truncate(m[s].length() - 1);
        w->formatOutput(_("已选择最接近的可用候补截止时间: %1").arg(m[s]));
        UserData::instance()->candidateSetting.selectedEndCandidateMinutes =
            jzdhDiffSelect[s].toInt();
        getCandidateQueueNum();
    } else {
        w->formatOutput(_("无可选截止时间，候补流程提前结束！"));
    }

}

void NetHelper::getCandidateQueueNum()
{
    QUrl url(_(CANDIDATEGETQUEUECOUNT));
    ReqParam param;

    post(url, param, &NetHelper::getCandidateQueueNumReply);
}

void NetHelper::getCandidateQueueNumReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handlecandidateError();
        return;
    }
    qDebug() << varMap;

    QVariantMap data = varMap[_("data")].toMap();
    QVariantList queueNum = data[_("queueNum")].toList();
    if (!queueNum.isEmpty()) {
        QVariantMap d = queueNum[0].toMap();
        QString queue_info = d[_("queue_info")].toString();
        if (!queue_info.isEmpty()) {
            QString seat_type_code = d[_("seat_type_code")].toString();
            QString train_code = d[_("station_train_code")].toString();
            QString train_date = d[_("train_date")].toString();
            w->formatOutput(_("车次：%1, 乘车日期：%2, 席别：%3 %4")
                                .arg(train_code, train_date,
                                     seatTypeSubmtiCodeTransToDesc(seat_type_code[0].toLatin1()),
                                     queue_info));
        }
    }
    // 额外的候补日期
    UserData *ud = UserData::instance();
    QString extraDate;
    QString extraDate2;
    bool hasExtraDate = false;
    QString curDate = ud->getUserConfig().tourDate;
    if (ud->candidateSetting.extraCandidateDateEn) {
        if (ud->candidateSetting.extraCandidateDate != curDate) {
            extraDate = ud->candidateSetting.extraCandidateDate;
            hasExtraDate = true;
        }
    }
    if (ud->candidateSetting.extraCandidateDate2En) {
        if (ud->candidateSetting.extraCandidateDate2 != curDate &&
            ud->candidateSetting.extraCandidateDate2 != ud->candidateSetting.extraCandidateDate) {
            extraDate2 = ud->candidateSetting.extraCandidateDate2;
            hasExtraDate = true;
        }
    }
    if (!hasExtraDate) {
        confirmHB();
    } else {
        struct CandidateDateInfo dInfo;
        dInfo.hasUpdate = false;
        if (!extraDate.isEmpty()) {
            dInfo.date = extraDate;
            ud->candidateInfo.diffDateTrain.append(dInfo);
            queryDiffDateTicket(extraDate);
        }
        if (!extraDate2.isEmpty()) {
            dInfo.date = extraDate2;
            ud->candidateInfo.diffDateTrain.append(dInfo);
            queryDiffDateTicket(extraDate2);
        }
        ud->candidateRunStatus = EWAITDIFFDATEDATA;
    }
}

QString NetHelper::generateCandidateTicketInfo()
{
    const QList<QString> &selectedPassengerList = w->passengerDialog->getSelectedPassenger();
    int i, k;
    UserData *ud = UserData::instance();
    QString secretList;
    QString tmp, passType;

    ud->candidateInfo.passengers.clear();
    for (i = 0; i < selectedPassengerList.size(); i++) {
        // 票类型#姓名#证件类型#证件号#EncStr#0;
        for (k = 0; k < ud->passenger.size(); k++) {
            if (ud->passenger[k].passName == selectedPassengerList[i]) {
                break;
            }
        }
        if (k < ud->passenger.size()) {
            passType = ud->grabSetting.isStudent ? ud->passenger[k].passType :
                           ud->passenger[k].passType == _("3") ? PASSENGERADULT : ud->passenger[k].passType;
            tmp.append(passType + '#');
            tmp.append(ud->passenger[k].passName.toUtf8().toPercentEncoding());
            tmp.append('#');
            tmp.append(ud->passenger[k].passIdTypeCode);
            tmp.append('#');
            tmp.append(ud->passenger[k].passIdNo);
            tmp.append('#');
            tmp.append(ud->passenger[k].allEncStr);
            tmp.append(_("#0;"));
        } else {
            continue;
        }
        secretList.append(tmp.toUtf8().toPercentEncoding());
        tmp.clear();
        ud->candidateInfo.passengers.append(ud->passenger[i].passName);
    }
    return secretList;
}

void NetHelper::confirmHB()
{
    QUrl url(_(CONFIRMHB));
    ReqParam param;
    UserData *ud = UserData::instance();

    param.put(_("passengerInfo"), generateCandidateTicketInfo());
    param.put(_("jzParam"), _(""));
    param.put(_("hbTrain"), _(""));
    param.put(_("lkParam"), _(""));
    param.put(_("sessionId"), _(""));
    param.put(_("sig"), _(""));
    param.put(_("scene"), _("nc_login"));
    param.put(_("encryptedData"), _(""));
    param.put(_("if_receive_wseat"), ud->candidateSetting.acceptNoSeat ? _("Y") : _("N"));
    param.put(_("realize_limit_time_diff"), QString::number(ud->candidateSetting.
                                                            selectedEndCandidateMinutes));
    int i, j, k;
    int count = 0;
    QString plans;
    QString seatType;
    QString acceptTrainDate;
    QString acceptTrainTime;
    QVector<QString> v;

    QString dispDate;
    QString dispTrain;
    QString dispSeat;
    QSet<QChar> seatCode;
    int totalTrain = 0;
    int totalSeat = 0;
    for (i = 0; i < ud->candidateInfo.diffDateTrain.size(); i++) {
        struct CandidateDateInfo &trainDate = ud->candidateInfo.diffDateTrain[i];
        dispDate += trainDate.date + ',';
    }
    if (!dispDate.isEmpty()) {
        dispDate.truncate(dispDate.size() - 1);
    }
    if (!ud->candidateInfo.diffDateTrain.isEmpty()) {
        struct CandidateDateInfo &trainDate = ud->candidateInfo.diffDateTrain[0];
        for (j = 0; j < trainDate.train.size(); j++) {
            if (j < 5) {
                dispTrain += trainDate.train[j].trainCode + ',';
            } else {
                break;
            }
        }
        totalTrain = trainDate.train.size() < 60 ? trainDate.train.size() : 60;
        if (!dispTrain.isEmpty()) {
            dispTrain.truncate(dispTrain.size() - 1);
        }
        if (!trainDate.train.isEmpty()) {
            for (k = 0; k < trainDate.train[0].seatType.size(); k++) {
                if (k < 3) {
                    dispSeat += seatTypeSubmtiCodeTransToDesc(trainDate.train[0].seatType[k]) + ',';
                }
                if (!seatCode.contains(trainDate.train[0].seatType[k])) {
                    seatCode.insert(trainDate.train[0].seatType[k]);
                }
            }
            totalSeat = seatCode.size();
            if (!dispSeat.isEmpty()) {
                dispSeat.truncate(dispSeat.size() - 1);
            }
        }
    }
    w->formatOutput(_("正在确认候补日期%1的%2共%3趟车次, %4共%5个席别").arg(dispDate, dispTrain)
                        .arg(totalTrain)
                        .arg(dispSeat)
                        .arg(totalSeat));

    for (i = 0; i < ud->candidateInfo.diffDateTrain.size(); i++) {
        struct CandidateDateInfo &trainDate = ud->candidateInfo.diffDateTrain[i];
        for (j = 0; j < trainDate.train.size(); j++) {
            for (k = 0; k < trainDate.train[j].seatType.size(); k++) {
                // 第0个是前面已经提交过的
                if (i == 0 && j == 0 && k == 0)
                    continue;

                // secretStr,seatTypeCode#
                if (count < 60) {
                    plans.append(trainDate.train[j].secretStr);
                    plans.append(',');
                    plans.append(trainDate.train[j].seatType[k]);
                    plans.append('#');
                }
                count++;
            }
        }
        if (j) {
            QString date = trainDate.date;
            v.append(date.remove('-'));
        }
    }

    if (!ud->candidateInfo.diffDateTrain.isEmpty() &&
        !ud->candidateInfo.diffDateTrain[0].train.isEmpty()) {
        seatType.append(ud->candidateInfo.diffDateTrain[0].train[0].seatType);
    }
    if (ud->candidateSetting.acceptNewTrain) {
        std::sort(v.begin(), v.end());
        for (auto &d : v) {
            acceptTrainDate.append(_("%1#").arg(d));
            acceptTrainTime.append(_("%1%2#").arg(ud->candidateSetting.extraCandidateStartHour, 2, 10, QChar('0'))
                                             .arg(ud->candidateSetting.extraCandidateEndHour, 2, 10, QChar('0')));
        }
    }
    // 备选日期的列车
    param.put(_("plans"), plans.toUtf8().toPercentEncoding());
    // 增开列车
    param.put(_("tmp_train_date"), acceptTrainDate.toUtf8().toPercentEncoding());
    param.put(_("tmp_train_time"), acceptTrainTime.toUtf8().toPercentEncoding());
    param.put(_("add_train_flag"), ud->candidateSetting.acceptNewTrain && !v.isEmpty() ? "Y" : _("N"));
    // 增开列车座位
    param.put(_("add_train_seat_type_code"), seatType);
    qDebug() << param.get();
    post(url, param, &NetHelper::confirmHBReply);
}

void NetHelper::confirmHBReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handlecandidateError();
        return;
    }
    qDebug() << __FUNCTION__;
    qDebug() << varMap;

    bool status = varMap[_("status")].toBool();
    if (!status) {
        QStringList messages = varMap[_("messages")].toStringList();
        if (!messages.isEmpty()) {
            w->formatOutput(messages[0]);
        }
        handlecandidateError();
        return;
    }
    QVariantMap data = varMap[_("data")].toMap();
    if (data.isEmpty()) {
        w->formatOutput(_("系统忙，请稍后再试！"));
        handlecandidateError();
        return;
    }
    QString msg = data[_("msg")].toString();
    if (!msg.isEmpty()) {
        w->formatOutput(msg);
    }
    bool isAsync = data[_("isAsync")].toBool();
    bool flag = data[_("flag")].toBool();
    if (isAsync) {
        if (flag) {
            candidateQueryQueue();
        } else {
            if (!msg.isEmpty()) {
                w->formatOutput(msg);
            }
            handlecandidateError();
        }
    } else {
        if (flag) {
            if (!msg.isEmpty()) {
                w->formatOutput(msg);
            } else {
                w->formatOutput(_("系统错误"));
            }
        }
        handlecandidateError();
    }
}

void NetHelper::candidateQueryQueue()
{
    QUrl url(_(CANDIDATEQUERYQUEUE));
    ReqParam param;

    post(url, param, &NetHelper::candidateQueryQueueReply);
}

void NetHelper::candidateQueryQueueReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        handlecandidateError();
        return;
    }
    qDebug() << __FUNCTION__;
    qDebug() << varMap;

    QVariantMap data = varMap[_("data")].toMap();
    if (data.isEmpty()) {
        w->formatOutput(_("系统忙，请稍后再试！"));
        handlecandidateError();
        return;
    }
    int status = data[_("status")].toInt();
    QString msg = data[_("msg")].toString();
    if (!msg.isEmpty()) {
        w->formatOutput(msg);
    }
    bool isAsync = data[_("isAsync")].toBool();
    bool flag = data[_("flag")].toBool();
    if (isAsync) {
        if (flag) {
            if (status == 1) {
                // htmlHref.lineUpPayConfirm
                if (queryCandidateTimer) {
                    queryCandidateTimer->stop();
                }
                QString reserve_no = data[_("reserve_no")].toString();
                lineUptoPayConfirm(reserve_no);
            } else if (status == -1) {
                w->formatOutput(!msg.isEmpty() ? msg : _("排队失败！"));
                handlecandidateError();
                if (queryCandidateTimer) {
                    queryCandidateTimer->stop();
                }
            } else {
                int waitTime = data[_("waitTime")].toInt();
                int mins = waitTime / 60;
                int secs = waitTime % 60;
                w->formatOutput(_("12306系统正在处理中，预计等待时间%1").arg(mins > 0 ?
                                                       _("%1分%2秒").arg(mins).arg(secs) :
                                                       _("%1秒").arg(secs)));
                if (!queryCandidateTimer) {
                    queryCandidateTimer = new QTimer;
                    connect(queryCandidateTimer, &QTimer::timeout, this, [=] {
                        candidateQueryQueue();
                    });
                }
                queryCandidateTimer->setInterval(mins > 0 ? 10000 : 1000);
                queryCandidateTimer->start();
            }
        } else {
            w->formatOutput(!msg.isEmpty() ? msg : _("系统错误"));
            handlecandidateError();
            if (queryCandidateTimer) {
                queryCandidateTimer->stop();
            }
        }
    } else {
        if (flag) {
            // htmlHref.lineUpPayConfirm
            QString reserve_no = data[_("reserve_no")].toString();
            lineUptoPayConfirm(reserve_no);
        } else {
            w->formatOutput(!msg.isEmpty() ? msg : _("系统错误"));
            handlecandidateError();
        }
        if (queryCandidateTimer) {
            queryCandidateTimer->stop();
        }
    }
    UserData *ud = UserData::instance();
    ud->candidateInfo.diffDateTrain.clear();
    ud->candidateInfo.submitSecretStr.clear();
    ud->candidateInfo.allSeatType.clear();
}

void NetHelper::sendMail()
{
    UserData *ud = UserData::instance();
    if (ud->notifySetting.emailNotify.notifyEmailEnable) {
        QString mailTemplate = "<html>"
                               "<head>"
                               "<meta charset=\"UTF-8\">"
                               "</head>"
                               "<body>"
                               "<p>恭喜您预订的车次%1已经出票成功，请您及时支付，否则订单超时将自动取消，订单详情如下：</p>"
                               "<br />"
                               "<table border=\"1\" style=\"color: #8B1A1A; background-color: #FFFAFA; margin-left: 20px\">"
                               "<tr>"
                               "<td>乘客</td><td>下单时间</td><td>乘车日期</td><td>车次</td><td>席别</td><td>始发站</td><td>终点站</td>"
                               "<td>出发站</td><td>到达站</td><td>出发时间</td><td>到达时间</td><td>历时</td>"
                               "</tr>";
        QString mailContent;
        QDateTime now = QDateTime::currentDateTime();
        mailContent += mailTemplate.arg(ud->submitTicketInfo.trainCode);
        for (int i = 0; i < ud->submitTicketInfo.submitSeatType.size(); i++) {
            mailContent += _("<tr>");
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.submitSeatType[i].first);
            mailContent += _("<td>%1</td>").arg(now.toString(_("yyyy-MM-dd hh:mm:ss")));
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.date);
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.trainCode);
            mailContent += _("<td>%1</td>").arg(seatTypeSubmtiCodeTransToDesc(ud->submitTicketInfo.submitSeatType[i].second));
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.startSTationName);
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.endStationName);
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.fromStationName);
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.toStationName);
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.fromTime);
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.toTime);
            mailContent += _("<td>%1</td>").arg(ud->submitTicketInfo.travelTime);
            mailContent += _("</tr>");
        }
        mailContent += "</table>"
            "<p>您收到此通知是因为您在<a href=\"https://www.op9.top\">云映程序</a>配置了此邮箱，"
            "如您未使用<a href=\"https://www.op9.top\">云映程序</a>进行过相关配置或未授权他人使用此邮箱，请忽略本邮件。</p>"
            "</body>"
            "</html>";
        w->settingDialog->sendMail(mailContent);
    }
}

void NetHelper::sendCandidateMail()
{
    UserData *ud = UserData::instance();
    if (ud->notifySetting.emailNotify.notifyEmailEnable) {
        QString mailTemplate = "<html>"
                               "<head>"
                               "<meta charset=\"UTF-8\">"
                               "</head>"
                               "<body>"
                               "<p>恭喜您候补车票已经出票成功，请您及时支付，否则订单超时将自动取消，订单详情如下：</p>"
                               "<br />"
                               "<table border=\"1\" style=\"color: #8B1A1A; background-color: #FFFAFA; margin-left: 20px\">"
                               "<tr>"
                               "<td>乘客</td><td>下单时间</td><td>乘车日期</td><td>车次</td><td>席别</td>"
                               "</tr>";
        QString mailContent;
        QDateTime now = QDateTime::currentDateTime();
        mailContent += mailTemplate;
        QString text;
        for (int i = 0; i < ud->candidateInfo.passengers.size(); i++) {
            mailContent += _("<tr>");
            mailContent += _("<td>%1</td>").arg(ud->candidateInfo.passengers[i]);
            mailContent += _("<td>%1</td>").arg(now.toString(_("yyyy-MM-dd hh:mm:ss")));
            mailContent += _("<td>");

            for (auto &trainDate : ud->candidateInfo.diffDateTrain) {
                text += trainDate.date + ("、");
            }
            if (!text.isEmpty()) {
                text.truncate(text.length() - 1);
            }
            mailContent += _("</td>");
            text.clear();
            for (auto &trainAtDate : ud->candidateInfo.diffDateTrain) {
                for (auto &train : trainAtDate.train) {
                    if (!text.contains(train.trainCode))
                        text += train.trainCode + _("、");
                }
            }
            if (!text.isEmpty()) {
                text.truncate(text.length() - 1);
            }
            mailContent += _("<td>%1</td>").arg(text);
            text.clear();
            for (auto &seatType : ud->candidateInfo.allSeatType) {
                text += seatTypeSubmtiCodeTransToDesc(seatType) + _("、");
            }
            if (!text.isEmpty()) {
                text.truncate(text.length() - 1);
            }
            mailContent += _("<td>%1</td>").arg(text);
            mailContent += _("</tr>");
            text.clear();
        }
        mailContent += "</table>"
                       "<p>您收到此通知是因为您在<a href=\"https://www.op9.top\">云映程序</a>配置了此邮箱，"
                       "如您未使用<a href=\"https://www.op9.top\">云映程序</a>进行过相关配置或未授权他人使用此邮箱，请忽略本邮件。</p>"
                       "</body>"
                       "</html>";
        w->settingDialog->sendMail(mailContent);
    }
}

void NetHelper::lineUptoPayConfirm(const QString &reserve_no)
{
    UserData *ud = UserData::instance();
    ud->candidateRunStatus = ESUBMITCANDIDATESUCCESS;
    QString msg;
    if (ud->paySetting.activePay) {
        msg = _("候补订单提交成功，候补订单号：%1");
    } else {
        msg = _("候补订单提交成功，候补订单号：%1，为避免订单超时取消，请您尽快完成支付").arg(reserve_no);
    }

    w->formatOutput(msg);
    // htmlHref.lineUpPayConfirm
    //QDesktopServices::openUrl(QUrl(_(LINEUPTOPAYCONFIRM)));
    // 结束任务
    sendCandidateMail();
    w->settingDialog->sendWxNotify(msg);

    if (ud->candidateSetting.onlyCandidate) {
        ud->lastRunSuccess = true;
        w->startOrStopGrabTicket();
        if (ud->generalSetting.playMusic) {
            w->startOrStopPlayMusic();
        }
    } else {
        ud->candidateSetting.isCandidate = false;
        w->settingDialog->closeCandidate();
        ud->setRunStatus(EGRABTICKET);
        ud->candidateRunStatus = EIDLE;
    }
    // 支付
    if (ud->paySetting.activePay) {
        getMyPublicIp();
        cpayOrderInit();
    }
}

// 起售查询
void NetHelper::queryScName(const QString &staCode)
{
    QUrl url(_(QUERYSCNAME));
    ReqParam param;
    param.put(_("station_telecode"), staCode);
    post(url, param, &NetHelper::queryScNameReply);
}

void NetHelper::queryScNameReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    w->selltimeDialog->queryScNameReply(varMap);
}

void NetHelper::queryAllCacheSaleTime()
{
    QUrl url(_(QUERYALLCACHESALETIME));
    ReqParam param;
    post(url, param, &NetHelper::queryAllCacheSaleTimeReply);
}

void NetHelper::queryAllCacheSaleTimeReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    w->selltimeDialog->queryAllCacheSaleTimeReply(varMap);
}

void NetHelper::queryLcTime(const QString &staCode, const QString &date)
{
    QUrl url(_(QUERYLCTIME));
    ReqParam param;
    param.put(_("from_station_telecode"), staCode);
    param.put(_("trainDate"), date);
    post(url, param, &NetHelper::queryLcTimeReply);
}

void NetHelper::queryLcTimeReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    w->selltimeDialog->queryLcTimeReply(varMap);
}

// 通知
void NetHelper::sendWxNotify(const QString &sendKey, const QString &msg)
{
    QUrl url(_("https://sctapi.ftqq.com/") + sendKey + _(".send"));
    ReqParam param;
    param.put(_("title"), _("云映状态通知").toUtf8().toPercentEncoding());
    param.put(_("desp"), msg.toUtf8().toPercentEncoding());
    param.put(_("short"), _("您有一条需要关注的消息").toUtf8().toPercentEncoding());
    anyPost(url, param, &NetHelper::sendWxNotifyReply);
}

void NetHelper::sendWxNotifyReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        w->formatOutput(_("消息发送失败：未知原因"));
        return;
    }

    w->settingDialog->sendWxNotifyReply(varMap);
}

void NetHelper::queryWxNotifyStatus(const QString &pushId, const QString &readKey)
{
    QUrl url(_("https://sctapi.ftqq.com/push?id=") + pushId + _("&readkey=") + readKey);
    anyGet(url, &NetHelper::queryWxNotifyStatusReply);
}

void NetHelper::queryWxNotifyStatusReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        w->formatOutput(_("消息发送失败：未知原因"));
        return;
    }
    w->settingDialog->queryWxNotifyStatusReply(varMap);
}

void NetHelper::getMyPublicIp()
{
    QUrl url(_("https://ifconfig.me/ip"));
    anyGet(url, &NetHelper::getMyPublicIpReply);
}

void NetHelper::getMyPublicIpReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        publicIp = varMap[_("data")].toString();
    }

    qDebug() << "public ip: " << publicIp;
}


// 支付
void NetHelper::queryNoCompleteOrder()
{
    QUrl url(_(QUERYNOCOMPLETEORDER));
    ReqParam param;

    param.put(_("_json_att"), _(""));

    post(url, param, &NetHelper::queryNoCompleteOrderReply);
    w->formatOutput(_("正在查询待支付订单..."));
}

void NetHelper::queryNoCompleteOrderReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    QVariantMap data = varMap[_("data")].toMap();
    QVariantList dbList = data[_("orderDBList")].toList();
    if (!dbList.isEmpty()) {
        QVariantMap itemMap = dbList[0].toMap();
        QString sequenceNo = itemMap[_("sequence_no")].toString();
        QVariantList ticketsList = itemMap[_("tickets")].toList();
        QString arriveTime;
        QVariantMap stationTrainMap;
        for (int i = 0; i < ticketsList.size(); i++) {
            stationTrainMap = ticketsList[i].toMap()[_("stationTrainDTO")].toMap();
            arriveTime += stationTrainMap[_("station_train_code")].toString() + ',' +
                   stationTrainMap[_("arrive_time")].toString() + ';';
        }
        payNoCompleteOrder(sequenceNo, arriveTime);
    } else {
        w->formatOutput(_("当前没有待支付的订单"));
    }
}

void NetHelper::payNoCompleteOrder(const QString &sequenceNo, const QString &arriveTime)
{
    QUrl url(_(PAYNOCOMPLETEORDER));
    ReqParam param;

    qDebug() << "sequence no: " << sequenceNo << " arrive time: " << arriveTime;
    param.put(_("sequence_no"), sequenceNo);
    param.put(_("pay_flag"), _("pay"));
    param.put(_("arrive_time_str"), arriveTime.toUtf8().toPercentEncoding());

    UserData *ud = UserData::instance();
    if (!ud->paySetting.activePay) {
        w->formatOutput(_("当前没有开启主动唤起支付，请先在'设置->支付设置'中开启主动唤起支付并选择支付方式"));
        return;
    }
    w->formatOutput(_("正在支付订单号为%1的订单...").arg(sequenceNo));
    post(url, param, &NetHelper::payNoCompleteOrderReply);
}

void NetHelper::payNoCompleteOrderReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }

    bool status = varMap[_("status")].toBool();
    if (!status) {
        qDebug() << "status is false";
        return;
    }
    QVariantMap data = varMap[_("data")].toMap();
    QString existError = data[_("existError")].toString();
    if (existError == _("Y")) {
        QString errorMsg = data[_("errorMsg")].toString();
        w->formatOutput(_("尝试支付时遇到错误：") + errorMsg);
        return;
    }
    payOrderInit();
}

void NetHelper::payOrderInit()
{
    QUrl url(_(PAYORDERINIT));
    ReqParam param;

    param.put(_("_json_att"), _(""));
    param.put(_("REPEAT_SUBMIT_TOKEN"), UserData::instance()->submitTicketInfo.repeatSubmitToken);

    post(url, param, &NetHelper::payOrderInitReply);
}

void NetHelper::payOrderInitReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        //QByteArray b = varMap[_("data")].toByteArray();
        //qDebug() << QString::fromUtf8(b);
    }
    getMyPublicIp();
    payCheckNew();
}

void NetHelper::payCheckNew()
{
    QUrl url(_(PAYCHECKNEW));
    ReqParam param;

    param.put(_("batch_nos"), _(""));
    param.put(_("coach_nos"), _(""));
    param.put(_("seat_nos"), _(""));
    param.put(_("passenger_id_types"), _(""));
    param.put(_("passenger_id_nos"), _(""));
    param.put(_("passenger_names"), _(""));
    param.put(_("allEncStr"), _(""));
    param.put(_("insure_price_all"), _(""));
    param.put(_("insure_types"), _(""));
    param.put(_("if_buy_insure_only"), _("N"));
    param.put(_("ins_selected_time"), _(""));
    param.put(_("ins_clause_time"), _(""));
    param.put(_("ins_notice_time"), _(""));
    param.put(_("hasBoughtIns"), _(""));
    param.put(_("ins_id"), _(""));
    param.put(_("_json_att"), _(""));

    post(url, param, &NetHelper::payCheckNewReply);
    w->formatOutput(_("正在主动唤起支付..."));
}

void NetHelper::payCheckNewReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    qDebug() << varMap;
    QVariantMap data = varMap[_("data")].toMap();
    payForm = data[_("payForm")].toMap();

    payWebBusiness();
}

void NetHelper::payGateway()
{
    //QUrl url(_(PAYGATEWAY));
    ReqParam param;

    QString epayUrl = payForm[_("epayurl")].toString();
    if (epayUrl.isEmpty()) {
        qDebug() << "epay url is empty";
        w->formatOutput(_("无法找到支付地址, 支付中止"));
        return;
    }
    param.put(_("_json_att"), _(""));
    param.put(_("interfaceName"), payForm[_("interfaceName")].toByteArray().toPercentEncoding());
    param.put(_("interfaceVersion"), payForm[_("interfaceVersion")].toByteArray().toPercentEncoding());
    param.put(_("tranData"), payForm[_("tranData")].toByteArray().toPercentEncoding());
    param.put(_("merSignMsg"), payForm[_("merSignMsg")].toByteArray().toPercentEncoding());
    param.put(_("appId"), payForm[_("appId")].toString());
    param.put(_("transType"), payForm[_("transType")].toString());

    qDebug() << __FUNCTION__;

    anyPost(QUrl(epayUrl), param, &NetHelper::payGatewayReply);
    qDebug() << param.get();
}

void NetHelper::payGatewayReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        //QByteArray b = varMap[_("data")].toByteArray();
        //qDebug() << QString::fromUtf8(b);
    }

    payWebBusiness();
}

void NetHelper::payWebBusiness()
{
    QUrl url(_(PAYWEBBUSINESS));
    ReqParam param;

    QString tranData = payForm[_("tranData")].toString();
    tranData.remove("\r\n");
    size_t len;
    char *data = (char *)decode_base64(tranData.toStdString().c_str(), tranData.size(), &len);
    data[len] = 0;
    QByteArray byteData(data);
    byteData.replace("GBK", "UTF-8");
    byteData.replace('\n', "");
    delete data;

    w->formatOutput(_("正在打开支付页面..."));
    QDomDocument doc("tranData");
    if (!doc.setContent(byteData)) {
        w->formatOutput(_("无法解析服务器返回的数据, 支付中止"));
        return;
    }
    QDomElement docElem = doc.documentElement();
    if (docElem.isNull()) {
        w->formatOutput(_("payForm为空, 支付中止"));
        return;
    }
    QDomNodeList nodeList = docElem.elementsByTagName(_("orderTimeoutDate"));
    if (nodeList.isEmpty()) {
        w->formatOutput(_("无法找到支付截止时间, 支付中止"));
        return;
    }
    QDomElement e = nodeList.item(0).toElement();
    qDebug() << "orderTimeoutDate: " << e.text();

    // 对应PayMethodE
    QStringList bankId = {
        _("33000010"),  // 支付宝
        _("33000020"),  // 微信支付
        _("00011000"),  // 中国银联
        _("01020000"),  // 工商银行
        _("01030000"),  // 农业银行
        _("01040000"),  // 中国银行
        _("01050000"),  // 建设银行
        _("03080000"),  // 招商银行
        _("01009999"),  // 邮储银行
        _("03010000"),  // 交通银行
        _("00011001"),  // 中铁银通卡
        _("00000000")  // VISA国际卡
    };
    UserData *ud = UserData::instance();
    int payMethod = ud->paySetting.payMethod < bankId.size() ? ud->paySetting.payMethod : 0;

    param.put(_("tranData"), payForm[_("tranData")].toByteArray().toPercentEncoding());
    param.put(_("transType"), payForm[_("transType")].toString());
    param.put(_("channelId"), _("1"));
    param.put(_("appId"), payForm[_("appId")].toString());
    param.put(_("merSignMsg"), payForm[_("merSignMsg")].toByteArray().toPercentEncoding());
    param.put(_("merCustomIp"), publicIp.toUtf8().toPercentEncoding());
    param.put(_("orderTimeoutDate"), e.text());
    param.put(_("paymentType"), _("0"));
    param.put(_("bankId"), bankId[payMethod]);  // 微信bankId
    param.put(_("businessType"), _("1"));
    qDebug() << __FUNCTION__;

    anyPost(url, param, &NetHelper::payWebBusinessReply);
    //qDebug() << param.get();
}

void NetHelper::payWebBusinessReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        QByteArray b = varMap[_("Content-Type")].toByteArray();
        if (b.startsWith("text/html")) {
            QByteArray data = varMap[_("data")].toByteArray();
            if (data.isEmpty()) {
                w->formatOutput(_("支付出错：服务器返回数据为空，请前往12306网站或12306手机APP完成支付"));
                return;
            }
            QTemporaryFile *tempFile = new QTemporaryFile(_("./cache/XXXXXX.html"));
            if (tempFile->open()) {
                int ret = tempFile->write(data);
                tempFile->close();
                if (ret > 0) {
                    if (QDesktopServices::openUrl(tempFile->fileName())) {
                        w->formatOutput("已在浏览器中打开支付页面，请在浏览器中完成支付");
                    } else {
                        w->formatOutput(_("打开支付页面失败，请前往12306网站或12306手机APP完成支付"));
                    }
                } else {
                    w->formatOutput(_("无法写入数据到文件，请前往12306网站或12306手机APP完成支付"));
                }

                tempFilesList.append(std::pair<QTemporaryFile *, int>(tempFile, 1000));
            } else {
                w->formatOutput(_("打开支付页面失败，请前往12306网站或12306手机APP完成支付"));
                delete tempFile;
            }
        }
        return;
    }
    //qDebug() << varMap;
}

// 候补支付
void NetHelper::cqueryQueue()
{
    ReqParam param;
    post(QUrl(_(CQUERYQUEUE)), param, &NetHelper::cqueryQueueReply);
    w->formatOutput(_("正在查询待支付候补订单..."));
}

void NetHelper::cqueryQueueReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }

    QVariantMap data = varMap[_("data")].toMap();
    bool flag = data[_("flag")].toBool();
    if (!flag) {
        w->formatOutput(_("当前没有待支付的候补订单"));
        return;
    }
    int status = data[_("status")].toInt();
    if (status != 1) {
        w->formatOutput(_("当前没有待支付的候补订单"));
        return;
    }
    QVariantMap order = data[_("order")].toMap();
    QString reserveNo = order[_("reserve_no")].toString();
    if (reserveNo.isEmpty()) {
        w->formatOutput(_("当前没有待支付的候补订单"));
        return;
    }
    w->formatOutput(_("查询到待支付的候补订单: ") + reserveNo);
    cpayNoCompleteOrder(reserveNo);
}

void NetHelper::cpayNoCompleteOrder(const QString &reserveNo)
{
    QUrl url(_(CPAYNOCOMPLETEORDER));
    ReqParam param;
    UserData *ud = UserData::instance();
    if (!ud->paySetting.activePay) {
        w->formatOutput(_("当前没有开启主动唤起支付，请先在'设置->支付设置'中开启主动唤起支付并选择支付方式"));
        return;
    }
    param.put(_("reserve_no"), reserveNo);
    post(url, param, &NetHelper::cpayNoCompleteOrderReply);
}

void NetHelper::cpayNoCompleteOrderReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    qDebug() << varMap;
    bool status = varMap[_("status")].toBool();
    if (status) {
        QVariantMap data = varMap[_("data")].toMap();
        bool flag = data[_("flag")].toBool();
        if (flag) {
            getMyPublicIp();
            cpayOrderInit();
        } else {
            w->formatOutput(_("候补订单支付失败: ") + data[_("msg")].toString());
        }
    } else {
        w->formatOutput(_("候补订单支付失败，请手动支付"));
    }
}

void NetHelper::cpayOrderInit()
{
    QUrl url(_(CPAYORDERINIT));
    ReqParam param;
    post(url, param, &NetHelper::cpayOrderInitReply);
}

void NetHelper::cpayOrderInitReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    qDebug() << varMap;
    cpayCheck();
}

void NetHelper::cpayCheck()
{
    QUrl url(_(CPAYCHECK));
    ReqParam param;
    post(url, param, &NetHelper::cpayCheckReply);
    w->formatOutput(_("正在主动唤起支付..."));
}

void NetHelper::cpayCheckReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        return;
    }
    qDebug() << varMap;
    QVariantMap data = varMap[_("data")].toMap();
    payForm = data[_("payForm")].toMap();

    payWebBusiness();
}


// 检查更新
void NetHelper::checkUpdate()
{
    QString platform;
#if defined(Q_OS_WIN)
    platform = "Windows";
#elif defined(Q_OS_MACOS)
    platform = "MACOS";
#else
    platform = "Linux";
#endif
    QUrl url(_(CHECKUPDATEURL "?ver=%1&platform=%2")
                 .arg(THISVERSION, platform));
    anyGet(url, &NetHelper::checkUpdateReply);
}

void NetHelper::checkUpdateReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        w->formatOutput(_("检查更新失败"));
        return;
    }
    //qDebug() << varMap;
    w->checkUpdateReply(varMap);
}

#ifdef HAS_CDN
void NetHelper::getCdn()
{
    QUrl url(_("http://101.201.78.185:8080/api/getcdn"));
    anyGet(url, &NetHelper::getCdnReply);
    w->formatOutput(_("正在从服务器获取cdn..."));
}

void NetHelper::getCdnReply(QNetworkReply *reply)
{
    QVariantMap varMap;
    if (replyIsOk(reply, varMap) < 0) {
        w->formatOutput(_("获取Cdn失败"));
        return;
    }
    int status = varMap[_("status")].toInt();
    if (status) {
        QString msg = varMap[_("msg")].toString();
        if (!msg.isEmpty()) {
            w->formatOutput(_("获取Cdn失败，描述：%1").arg(msg));
            return;
        }
    }
    QStringList cdnList = varMap[_("cdnList")].toStringList();
    if (!cdnList.isEmpty()) {
        cdn.addCdns(cdnList);
        cdn.startTest();
    }
     w->formatOutput(_("从服务器获取到%1个cdn").arg(cdnList.size()));

    //qDebug() << varMap;
}
#endif

NetHelper::~NetHelper()
{
    delete nam;
    delete nam2;
    delete capSnapTimer;
    delete rttDelayTimer;
    delete keepLoginTimer;
    for (int i = 0; i < tempFilesList.size(); i++) {
        delete tempFilesList[i].first;
    }
}
