#include "nethelper.h"
#include <QRandomGenerator>
#include <time.h>

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
    double randnum = QRandomGenerator::global()->generateDouble();
    QNetworkReply *reply = d->man->get(
                QNetworkRequest(
                    QUrl(
                        QString(
                            "https://kyfw.12306.cn/passport/captcha/captcha-image"
                            "?login_site=E&module=login&rand=sjrand&"
                            + QString::number(randnum)))));
    replyMap.insert(reply, EGETVARIFICATIONCODE);
}

void NetHelper::doLogin()
{

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
    request.setRawHeader("User-Agent",
                         "Mozilla/5.0 (X11; Fedora; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36");
    QNetworkReply *reply = d->man->get(request);
    replyMap.insert(reply, EQUERYTICKET);
}

void NetHelper::getStationNameTxt()
{
    QUrl url("https://kyfw.12306.cn/otn/resources/js/framework/station_name.js?station_version=1.9094");
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent",
                         "Mozilla/5.0 (X11; Fedora; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/61.0.3163.100 Safari/537.36");
    QNetworkReply *reply = d->man->get(request);
    replyMap.insert(reply, EGETSTATIONNAMETXT);
}
NetHelper::~NetHelper()
{
    delete d;
}
