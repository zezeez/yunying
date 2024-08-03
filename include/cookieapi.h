#ifndef COOKIEAPI_H
#define COOKIEAPI_H

#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QDir>
#include <userdata.h>

#define CACHEPATH "cache"
#define COOKIEFILE "/cookies"
#define _ QStringLiteral

class CookieAPI: public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit CookieAPI(QObject *parent = nullptr) : QNetworkCookieJar(parent)
    {
        QString cachePath = getAppCachePath();

        if (cachePath.isEmpty()) {
            qWarning() << "Could not open application data path";
            cachePath = "./cache";
            QDir dir;
            if (!dir.exists(cachePath)) {
                if (!dir.mkpath(cachePath)) {
                    qWarning() << "Could not create data directory:" << cachePath;
                    return;
                }
            }
        }
        QFile file(cachePath + _(COOKIEFILE));
        if (!file.open(QFile::ReadOnly)) {
            return;
        }
        QByteArray bytesRead = file.readLine(1024);
        QList<QNetworkCookie> allCookieList;
        QNetworkCookie cookieParse;
        QString domain;
        while (!bytesRead.isEmpty()) {
            bytesRead.truncate(bytesRead.length() - 1);
            QString cookieLine = QString::fromUtf8(bytesRead);
            if (cookieLine.startsWith(_("domain="))) {
                domain = cookieLine.sliced(7);
            } else {
                QList<QNetworkCookie> cookieList = cookieParse.parseCookies(bytesRead);
                for (auto &i : cookieList) {
                    i.setDomain(domain);
                }
                allCookieList.append(cookieList);
            }
            bytesRead = file.readLine(1024);
        }
        if (!allCookieList.empty()) {
            setAllCookies(allCookieList);
        }
        file.close();
    };

    virtual ~CookieAPI()
    {
        QString cachePath = getAppCachePath();

        if (cachePath.isEmpty()) {
            qWarning() << "Could not open application data path";
            cachePath = "./cache";
            QDir dir;
            if (!dir.exists(cachePath)) {
                if (!dir.mkpath(cachePath)) {
                    qWarning() << "Could not create data directory:" << cachePath;
                    return;
                }
            }
        }
        QFile file(cachePath + _(COOKIEFILE));
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            return;
        }
        QList<QNetworkCookie> cookieList = allCookies();
        QByteArray cookieHeader;
        QString saveDomain;
        QString domain;
        for (auto &i : cookieList) {
            domain = i.domain();
            if (domain != saveDomain) {
                cookieHeader.append(_("domain=%1\n").arg(domain).toUtf8());
                saveDomain = domain;
            }
            cookieHeader.append(_("%1; path=%2\n").arg(i.toRawForm(QNetworkCookie::NameAndValueOnly), i.path()).toUtf8());
        }
        if (cookieHeader.size()) {
            file.write(cookieHeader);
        }
        file.close();
    };
};

#endif // COOKIEAPI_H
