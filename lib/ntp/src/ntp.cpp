#include "lib/ntp/include/ntp.h"
#include "mainwindow.h"
#include <QDateTime>
#include <QCryptographicHash>
#include <QtEndian>
#include <QNetworkDatagram>
#if defined(Q_OS_WIN)
#include <WinSock2.h>
#define hton64 htonll
#define ntoh64 ntohll
#elif defined(Q_OS_MACOS)
#include <machine/endian.h>
#define hton64 htonll
#define ntoh64 ntohll
#else
#include <stdint.h>
#define hton64 htobe64
#define ntoh64 be64toh
#endif
#include "sysutil.h"

#define _ QStringLiteral

extern MainWindow *w;

Ntp::Ntp(QObject *parent) : QObject(parent)
{
    udp = nullptr;
}

Ntp::~Ntp()
{

}

void Ntp::setupUdp()
{
    if (!udp) {
        udp = new QUdpSocket;
        connect(udp, &QUdpSocket::readyRead,
                this, &Ntp::receive);
        connect(udp, &QUdpSocket::stateChanged,
                this, &Ntp::socketStateChanged);
    }
}

void Ntp::clearUdp()
{
    if (udp) {
        udp->disconnectFromHost();
        udp->deleteLater();
        udp = nullptr;
    }
}

void Ntp::syncTime(const QString &server)
{
    if (server.isEmpty()) {
        return;
    }
    serverV4 = server;
    setupUdp();
    udp->connectToHost(serverV4, NTPPORT);
}

QString Ntp::server()
{
    return serverV4;
}

quint64 ntpTime()
{
    QDateTime cur = QDateTime::currentDateTimeUtc();
    quint64 sec = cur.toSecsSinceEpoch();
    quint64 msec = cur.time().msec();
    return TONTPLL(sec, msec);
}

void Ntp::send()
{
    struct NtpPacket p;
    quint64 t;

    memset(&p, 0, sizeof(p));
    // Version 4
    p.mode |= 4 << 3;
    // client
    p.mode |= 3;
    p.stratum = 0;
    p.poll = 6;
    p.presicion = -16;
    t = ntpTime();
    p.org = hton64(t);
    org = t;
    //qToBigEndian<quint64>(&t, 1, &p.org);
    QByteArray data;
    data.append((const char *)&p, sizeof(p) - sizeof(p.digest));
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(data);
    QByteArray resultMd5 =hash.result();
    //qDebug() << resultMd5.toHex() << " " << resultMd5.size();
    //qToBigEndian<quint8>(resultMd5.data(), resultMd5.size() < 16 ? resultMd5.size(): 16, p.digest);
    quint64 buf[2];
    memcpy(buf, resultMd5.data(), resultMd5.size() < 16 ? resultMd5.size(): 16);
    for (int i = 0; i < 2; i++) {
        buf[i] = hton64(buf[i]);
    }
    memcpy(p.digest, buf, sizeof(p.digest));

    qint64 size = udp->write((const char *)&p, sizeof(p));
    if (size == -1) {
        qDebug() << "write error: " << udp->errorString();
    }

}

void Ntp::receive()
{
    struct NtpPacket *p;
    QByteArray reply = udp->readAll();
    if ((size_t)reply.size() < sizeof(*p) - sizeof(p->digest)) {
        qDebug() << "reply size " << reply.size() << " are too small.";
        return;
    }
    p = reinterpret_cast<struct NtpPacket *>(reply.data());
    if (p->stratum == 0) {
        // See https://www.rfc-editor.org/rfc/rfc5905 7.4 The Kiss-o'-Death Packet
        w->formatOutput(_("从服务器同步时间失败，服务器返回错误"));
        return;
    }

    quint64 org = p->org ? ntoh64(p->org) : this->org;
    quint64 cur = ntpTime();
    quint64 delay = ((cur - org) - (ntoh64(p->rec) - ntoh64(p->xmt))) / 2;
    quint64 sec = SECFROMNTPLL(ntoh64(p->xmt)) + (delay >> 32);
    quint64 msec = MSECFROMNTPLL(ntoh64(p->xmt)) + MSECFROMNTPLL(delay);
    QDateTime correctTime = QDateTime::fromMSecsSinceEpoch(sec * 1000 + msec);
    QDateTime sysTime = QDateTime::fromMSecsSinceEpoch(SECFROMNTPLL(cur) * 1000 + MSECFROMNTPLL(cur));
    qDebug() << "correct time: " << correctTime;
    w->formatOutput(_("当前系统时间: %1，服务器时间: %2").arg(sysTime.toString(_("yyyy-MM-dd hh:mm:ss.zzz")),
                                                          correctTime.toString(_("yyyy-MM-dd hh:mm:ss.zzz"))));
    qDebug() << _("当前系统时间: %1，服务器时间: %2").arg(sysTime.toString(_("yyyy-MM-dd hh:mm:ss.zzz")),
                                                          correctTime.toString(_("yyyy-MM-dd hh:mm:ss.zzz")));
    if (qAbs(sysTime.msecsTo(correctTime)) > 10) {
        w->formatOutput(_("正在更新系统时间..."));
        SysUtil::setSysTime(correctTime);
    }
    clearUdp();
}

void Ntp::socketStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "ntp: " << state;
    if (state == QAbstractSocket::ConnectedState) {
        send();
    }
}
