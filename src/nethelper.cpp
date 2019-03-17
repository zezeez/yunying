#include "nethelper.h"
#include <QRandomGenerator>
#include <time.h>
#include <QPoint>
#include <QUrl>

NetHelper::NetHelper(QObject *parent) : QObject(parent),
    nam(new QNetworkAccessManager(this))
{
    connect(nam, &QNetworkAccessManager::finished,
                this, &NetHelper::finished);
}

NetHelper *NetHelper::instance()
{
    static NetHelper nh;
    return &nh;
}

void NetHelper::post(const QUrl &url, const QString &data, enum NETREQUESTTYPEENUM type)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    QNetworkReply *reply = nam->post(request, data.toLocal8Bit());
    replyMap.insert(reply, type);
}

void NetHelper::get(const QUrl &url, enum NETREQUESTTYPEENUM type)
{
    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", USERAGENT);
    QNetworkReply *reply = nam->get(request);
    replyMap.insert(reply, type);
}

void NetHelper::getVarificationImage()
{
    QDateTime now = QDateTime::currentDateTime();
    QString urlStr = QString(
                CAPTCHAIMAGEURL
                "?login_site=E&module=login&rand=sjrand&" +
                QString::number(now.toMSecsSinceEpoch()));
    QUrl url(urlStr);
    get(url, EGETVARIFICATIONCODE);
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

    QUrl url(baseUrl + param);
    get(url, EDOVARIFICATION);
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

    post(url, data, ELOGIN);
}

void NetHelper::queryTicket(const QString &staStartCode, const QString &staEndCode, const QString &date)
{
    QUrl url;
    url.setUrl(OTNURL"/leftTicket/queryX"
               "?leftTicketDTO.train_date=" + date +
               "&leftTicketDTO.from_station=" + staStartCode +
               "&leftTicketDTO.to_station=" + staEndCode +
               "&purpose_codes=ADULT");

    get(url, EQUERYTICKET);
}

void NetHelper::passportUamtk()
{
    QUrl url(QStringLiteral(PASSPORTURL"/web/auth/uamtk"));
    QString data = QStringLiteral("appid=otn");

    post(url, data, EPASSPORTUAMTK);
}

void NetHelper::passportUamtkClient(QString apptk)
{
    QUrl url(QStringLiteral(BASEURL"/otn/uamauthclient"));
    QString data = QString("tk=%1").arg(apptk);

    post(url, data, EPASSPORTUAMTKCLIENT);
}

void NetHelper::userIsLogin()
{
    QUrl url(QStringLiteral(PASSPORTURL"/web/auth/uamtk-static"));
    QString data = QString("appid=otn");

    post(url, data, EQUERYLOGINSTATUS);
}

void NetHelper::getPassengerInfo()
{
    QUrl url(QStringLiteral(OTNURL"/confirmPassenger/getPassengerDTOs"));
    QString data = QString("_json_attr=");

    post(url, data, EGETPASSENGERINFO);
}

void NetHelper::passengerInitDc()
{
    QUrl url(QStringLiteral(BASEURL"/confirmPassenger/initDc"));
    QString data = QString("_json_attr=");

    post(url, data, EPASSENGERINITDC);
}

void NetHelper::checkUser()
{
    QUrl url(QStringLiteral(OTNURL"/login/checkUser"));
    QString data = QString("_json_att=");

    post(url, data, ECHECKUSER);
}

void NetHelper::submitOrderRequest(const QString &secStr, const QString &date,
                                   const QString fromStationName, const QString toStationName)
{
    QUrl url(QStringLiteral(OTNURL"/leftTicket/submitOrderRequest"));
    QString data = QString("secretStr=" + secStr + "&train_date=" + date +
                           "&back_train_date=" + date + "&tour_flag=dc&"
                           "purpose_codes=ADULT&query_from_station_name=" + fromStationName +
                           "&query_to_station_name=" + toStationName + "&undefined  ");
    qDebug() << "data = " << data << endl;

    post(url, data, ESUBMITORDERREQUEST);
}

void NetHelper::checkOrderInfo(const QString &passenger, const QString &oldPassenger)
{
    QUrl url(QStringLiteral(BASEURL"/confirmPassenger/checkOrderInfo"));
    QString data = QString("cancel_flag=2&bed_level_order_num="
                           "000000000000000000000000000000&"
                           "passengerTicketStr=%1&oldPassengerStr=%2&"
                           "tour_flag=dc&whatsSelect=1&randCode=&"
                           "whatsSelect=1")
            .arg(passenger)
            .arg(oldPassenger);

    post(url, data, ESUBMITORDERREQUEST);
}

void NetHelper::getStationNameTxt()
{
    QUrl url(QStringLiteral(OTNURL"/resources/js/framework/station_name.js?station_version=1.9094"));

    get(url, EGETSTATIONNAMETXT);
}

NetHelper::~NetHelper()
{
    delete nam;
}
