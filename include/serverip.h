#ifdef HAS_CDN
#ifndef SERVERIP_H
#define SERVERIP_H
#include <QUdpSocket>
#include <QObject>
#include <QSet>
#include <QTimer>

class ServerIp : public QObject
{
public:
    ServerIp(QObject *parent = nullptr);
    ~ServerIp();
    void setupUdp();
    void destoryUdp();
    void socketStateChanged(QAbstractSocket::SocketState state);
    void dnsAnswer(const QString hostName);
    void recvDnsAnswer();
private:
    QUdpSocket *udp;
    QSet<QString> serverIp;
    QTimer recycleTimer;
};

#endif // SERVERIP_H
#endif
