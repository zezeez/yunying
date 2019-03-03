#include "nethelper.h"
#include <QRandomGenerator>
#include <time.h>
#include <QPoint>
#include <QUrl>

NetHelper::NetHelper(QObject *parent) : QObject(parent),
    d(new NetHelper::NetHelperPrivate(this))
{
    connect(d->man, &QNetworkAccessManager::finished,
                this, &NetHelper::finished);
}

NetHelper *NetHelper::instance()
{
    static NetHelper nh;
    return &nh;
}

void NetHelper::get(const QString &url)
{
    d->man->get(QNetworkRequest(QUrl(url)));
}

void NetHelper::getVarificationImage()
{
    QDateTime now = QDateTime::currentDateTime();
    QString url = QString(
                CAPTCHAIMAGEURL
                "?login_site=E&module=login&rand=sjrand&" +
                QString::number(now.toMSecsSinceEpoch()));
    QNetworkReply *reply = d->man->get(
                QNetworkRequest(
                    QUrl(url
                        )));
    replyMap.insert(reply, EGETVARIFICATIONCODE);
}

void NetHelper::doVarification(QVector<QPoint> points)
{
    QString varCode;
    QString baseUrl = QStringLiteral(CAPTCHACHECKURL);
    QString param;
    QDateTime now = QDateTime::currentDateTime();

    for (int i = 0; i < points.size(); i++) {
        varCode += QString("%1,%2,").arg(points[i].x()).arg(points[i].y());
    }
    varCode = varCode.left(varCode.length() - 1);
    varCode.replace(",", "%2C");

    param = QString("?answer=" + varCode + "&rand=sjrand&login_site=E&_=" +
            QString::number(now.toMSecsSinceEpoch()));

    QNetworkRequest request;
    QUrl url(baseUrl + param);
    request.setUrl(url);
    QNetworkReply *reply = d->man->get(request);
    replyMap.insert(reply, EDOVARIFICATION);
}
void NetHelper::doLogin(QVector<QPoint> points, QString name, QString passwd)
{
    QString varCode;
    QUrl url(QStringLiteral(LOGINURL));
    QString data;

    for (int i = 0; i < points.size(); i++)
        varCode += QString("%1,%2").arg(points[i].x(), points[i].y());
    varCode = varCode.left(varCode.length() - 1);
    varCode.replace(",", "%2C");

    data = QString("username=%1&password=%2&appid=otn&answer=")
            .arg(name).arg(passwd);
    data += varCode;

    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = d->man->post(request, data.toLocal8Bit());
    replyMap.insert(reply, ELOGIN);
}

void NetHelper::queryTicket(const QString &staStartCode, const QString &staEndCode, const QString &date)
{
    QUrl url("https://kyfw.12306.cn/otn/leftTicket/log"
             "?leftTicketDTO.train_date=" + date +
             "&leftTicketDTO.from_station=" + staStartCode +
             "&leftTicketDTO.to_station=" + staEndCode +
             "&purpose_codes=ADULT");
    //d->man->get(QNetworkRequest(url));
    url.setUrl("https://kyfw.12306.cn/otn/leftTicket/queryX"
               "?leftTicketDTO.train_date=" + date +
               "&leftTicketDTO.from_station=" + staStartCode +
               "&leftTicketDTO.to_station=" + staEndCode +
               "&purpose_codes=ADULT");
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", USERAGENT);
    QNetworkReply *reply = d->man->get(request);
    replyMap.insert(reply, EQUERYTICKET);
}

void NetHelper::passportUamtk()
{
    QUrl url(QStringLiteral(PASSPORTURL"/web/auth/uamtk"));
    QString data = QStringLiteral("appid=otn");

    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = d->man->post(request, data.toLocal8Bit());
    replyMap.insert(reply, EPASSPORTUAMTK);
}

void NetHelper::passportUamtkClient(QString apptk)
{
    QUrl url(QStringLiteral(BASEURL"/otn/uamauthclient"));
    QString data = QString("tk=%1").arg(apptk);

    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = d->man->post(request, data.toLocal8Bit());
    replyMap.insert(reply, EPASSPORTUAMTKCLIENT);
}

void NetHelper::userIsLogin()
{
    QUrl url(QStringLiteral(PASSPORTURL"/web/auth/uamtk-static"));
    QString data = QString("appid=otn");

    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = d->man->post(request, data.toLocal8Bit());
    replyMap.insert(reply, EQUERYLOGINSTATUS);
}

void NetHelper::getStationNameTxt()
{
    QUrl url(QStringLiteral("https://kyfw.12306.cn/otn/resources/js/framework/station_name.js?station_version=1.9094"));
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", USERAGENT);
    QNetworkReply *reply = d->man->get(request);
    replyMap.insert(reply, EGETSTATIONNAMETXT);
}
NetHelper::~NetHelper()
{
    delete d;
}
