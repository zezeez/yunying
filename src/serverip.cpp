#ifdef HAS_CDN
#include "lib/dns/include/message.h"
#include "lib/dns/include/exception.h"
#include "lib/dns/include/rr.h"
#include "serverip.h"
#include "nethelper.h"
#include <QString>
#include <QStringList>
#include <QHostAddress>
#include <QNetworkDatagram>

#define DNSPORT 53
#define UDPBUFFSIZE 512

QStringList dnsServers = {
    "1.2.4.8",
    "8.8.8.8",
    "114.114.114.114",
    "223.6.6.6",  // 阿里
    "4.2.2.1",  // a.resolvers.level3.net.
    "61.132.163.68",  // 安徽电信
    "221.130.33.60",  // 北京移动
    "203.133.1.6",  // dns2.giga.net.tw.
    "119.29.29.29",
    "202.101.224.69",
    "221.130.33.60",
    "210.2.4.8"
};

ServerIp::ServerIp(QObject *parent) : QObject(parent)
{
    udp = new QUdpSocket;
    connect(udp, &QUdpSocket::readyRead,
            this, &ServerIp::recvDnsAnswer);
    connect(udp, &QUdpSocket::stateChanged,
            this, &ServerIp::socketStateChanged);
}

ServerIp::~ServerIp()
{
    delete udp;
}

void ServerIp::socketStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "ServerIp: " << state;
}

void ServerIp::dnsAnswer(const QString hostName)
{
    dns::Message m;
    m.setQr(dns::Message::typeQuery);
    // add NAPTR query
    dns::QuerySection *qs = new dns::QuerySection(hostName.toStdString());
    qs->setType(dns::RDATA_A);
    qs->setClass(dns::QCLASS_IN);
    m.addQuery(qs);
    int id = 0;
    char bufSend[UDPBUFFSIZE];

    for (auto &server : dnsServers) {
        m.setId(++id);

        uint msgSize;
        m.encode(bufSend, UDPBUFFSIZE, msgSize);

        udp->writeDatagram(bufSend, msgSize, QHostAddress(server), DNSPORT);
    }
}

void ServerIp::recvDnsAnswer()
{
    QNetworkDatagram data = udp->receiveDatagram();
    if (!data.isValid()) {
        qDebug() << "receive data not valid";
        return;
    }
    QByteArray reply = data.data();
    dns::Message m;
    try {
        m.decode(reply.data(), reply.size());
        //qDebug() << m.asString();
        std::vector<dns::ResourceRecord*> answers = m.getAnswers();

        for(std::vector<dns::ResourceRecord*>::iterator it = answers.begin(); it != answers.end(); ++it) {
            if ((*it)->getType() == dns::RDATA_A) {
                QString h = QString::fromStdString((*it)->asString());
                if (!serverIp.contains(h)) {
                    NetHelper::instance()->cdn.addAvaliableCdn(h);
                    serverIp.insert(h);
                    qDebug() << "add ava host: " << h;
                }
            }
        }
    }
    catch (dns::Exception& e) {
        qDebug() << "DNS exception occured when parsing incoming data: " << e.what();
    }
}
#endif
