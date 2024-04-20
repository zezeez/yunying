#ifdef HAS_CDN
#ifndef SERVERIP_H
#define SERVERIP_H
#include <QUdpSocket>
#include <QObject>
#include <QSet>

class ServerIp : public QObject
{
public:
    ServerIp(QObject *parent = nullptr);
    ~ServerIp();
    void socketStateChanged(QAbstractSocket::SocketState state);
    void dnsAnswer(const QString hostName);
    void recvDnsAnswer();
private:
    QUdpSocket *udp;
    QSet<QString> serverIp;
};

#endif // SERVERIP_H
#endif
