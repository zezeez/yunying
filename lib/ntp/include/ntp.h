#ifndef NTP_H
#define NTP_H
#include <QUdpSocket>
#include <QObject>

#define JAN_1970        2208988800UL /* 1970 - 1900 in seconds */
#define FRAC       4294967296.             /* 2^32 as a double */
#define TONTPLL(s, ms)  ((quint64)(((s) + JAN_1970) << 32) + (quint64)((ms) / 1e3 * FRAC))
#define SECFROMNTPLL(n) ((((quint64)(n) >> 32)) - JAN_1970)
#define MSECFROMNTPLL(n) ((((quint64)(n) & 0xffffffff) * 1000) >> 32)
#define NTPPORT 123

#pragma pack(push, 1)

struct NtpPacket {
    // See rfc5905
    unsigned char mode;
    /*
     * +--------+-----------------------------------------------------+
        | Value  | Meaning                                             |
        +--------+-----------------------------------------------------+
        | 0      | unspecified or invalid                              |
        | 1      | primary server (e.g., equipped with a GPS receiver) |
        | 2-15   | secondary server (via NTP)                          |
        | 16     | unsynchronized                                      |
        | 17-255 | reserved                                            |
        +--------+-----------------------------------------------------+
    */
    quint8 stratum;  // Stratum (stratum): 8-bit integer representing the stratum
    qint8 poll;  // Poll: 8-bit signed integer representing the maximum interval between successive messages, in log2 seconds.
    qint8 presicion;  // Precision: 8-bit signed integer representing the precision of the system clock, in log2 seconds.
    quint32 rootDelay;  // Root Delay (rootdelay): Total round-trip delay to the reference clock, in NTP short format.
    quint32 rootDisp;  // Root Dispersion (rootdisp): Total dispersion to the reference clock, in NTP short format.
    quint32 refId;  //  Reference ID (refid): 32-bit code identifying the particular server or reference clock.
    quint64 refTimestamp;  // Reference Timestamp: Time when the system clock was last set or corrected, in NTP timestamp format.
    quint64 org;  // Origin Timestamp (org): Time at the client when the request departed for the server, in NTP timestamp format.
    quint64 rec;  // Receive Timestamp (rec): Time at the server when the request arrived from the client, in NTP timestamp format.
    quint64 xmt;  // Transmit Timestamp (xmt): Time at the server when the request arrived from the client, in NTP timestamp format.
    quint32 keyId;  //  Key Identifier (keyid): 32-bit unsigned integer used by the client and server to designate a secret 128-bit MD5 key.
    quint8 digest[16];  // Message Digest (digest): 128-bit MD5 hash computed over the key followed by the NTP packet header and extensions fields
                             // (but not the Key Identifier or Message Digest fields).
};

#pragma pack(pop)

class Ntp : public QObject
{
    Q_OBJECT
public:
    Ntp(QObject *parent = nullptr);
    ~Ntp();
    void setupUdp();
    void clearUdp();
    QString server();
    void send();
    void receive();
    void syncTime(const QString &server);
    void socketStateChanged(QAbstractSocket::SocketState state);
private:
    QString serverV4;
    quint64 org;
    QUdpSocket *udp;
};
#endif // NTP_H
