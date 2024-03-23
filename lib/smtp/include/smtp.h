#pragma once
#include <string>
#include <vector>
#include <map>

#include <QTcpSocket>
#include <QSslSocket>
#include <functional>
#include <QObject>
#include <queue>

class SmtpBase : public QObject
{
    Q_OBJECT
protected:
    struct EmailInfo
    {
        std::string smtpServer;      //the SMTP server
        std::string serverPort;      //the SMTP server port
        std::string charset;         //the email character set
        std::string sender;          //the sender's name
        std::string senderEmail;     //the sender's email
        std::string password;        //the password of sender
        std::string recipient;       //the recipient's name
        std::string recipientEmail;  //the recipient's email

        std::vector<std::string> recvList; //收件人列表<email, name>

        std::string subject;         //the email message's subject  邮件主题
        std::string message;         //the email message body   邮件内容

        std::vector<std::string> ccEmail;         //抄送列表
        std::vector<std::string> attachment; //附件
    };
public:
    SmtpBase(QObject *parent = nullptr) : QObject(parent)
    {

    }
    virtual ~SmtpBase() {}
    /**
     * @brief 简单发送文本邮件
     * @param   from 发送者的帐号
     * @param   passs 发送者密码
     * @param   to 收件人
     * @param   subject 主题
     * @param   strMessage  邮件内容
     */

    virtual int sendEmail(const std::string& from, const std::string& passs, const std::string& to, const std::string& subject, const std::string& strMessage) = 0;
    /**
     * @brief 发送邮件，包括附件以及抄送人和多个收件人
     * @param   from 发送者的帐号
     * @param   passs 发送者密码
     * @param   vecTo 收件人列表
     * @param   subject 主题
     * @param   strMessage  邮件内容
     * @param   attachment  附件列表    附件可以是绝对路径，默认是可执行程序目录下
     * @param   ccList  抄送列表
     */
    virtual int sendEmail(const std::string& from, const std::string& passs, const std::vector<std::string>& vecTo,
        const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment,const std::vector<std::string>& ccList) = 0;

protected:
    std::queue<struct EmailInfo> emailQueue;
    int curReceiver;
};

enum SMTPSTATUS {
    EERROR = -1,
    //EINIT = 0,
    EHELO = 0,
    ETOAUTH,
    EMAILFROM,
    ERCPTO,
    EDATA,
    EMAILBODY,
    EEND,
    ERESET,
    EQUIT,
};

enum MAILSTATE {
    EMAILSUCCESS = 0,
    EMAILFAILED
};

typedef std::string (*func)();
class SmtpEmail : public SmtpBase
{
    Q_OBJECT
public:
    SmtpEmail(QObject *parent = nullptr);
    virtual ~SmtpEmail();

    virtual int sendEmail(const std::string& from, const std::string& passwd, const std::string& to, const std::string& subject, const std::string& strMessage);

    virtual int sendEmail(const std::string& from, const std::string& passwd, const std::vector<std::string>& vecTo,
        const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment, const std::vector<std::string>& ccList);
    std::pair<enum SMTPSTATUS, std::string> smtpProtocolRun(const std::string &input, std::string &error);

    std::string getEmailBody();
    bool mailSuccess();
Q_SIGNALS:
    void mailStateNotify(enum MAILSTATE, QString error, QString errorDetail);

private:

    enum SMTPSTATUS status;
    bool isSuccess;
    std::vector<std::function<std::pair<enum SMTPSTATUS, std::string>()>> nextStatus;
#define EXITCODE "221"
    std::map<std::string, std::string> okCode;
    std::map<std::string, std::string> errCode;
};

class SimpleSmtpEmail : public SmtpEmail
{
    Q_OBJECT
public:
    SimpleSmtpEmail(QObject *parent = nullptr);
    virtual ~SimpleSmtpEmail();
    virtual int sendEmail(const std::string& from, const std::string& passwd, const std::string& to, const std::string& subject, const std::string& strMessage);

    virtual int sendEmail(const std::string& from, const std::string& passwd, const std::vector<std::string>& vecTo,
                          const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment,
                          const std::vector<std::string>& ccList);
    void connectToHost();
    void setHost(const QString &host);
    void setPort(qint16 port);
    void setSsl(bool ssl);
    void disconnect();
    void readyRead();
    void socketError();
    void sslError();
    void socketEncrypted();
    void socketStateChanged(QAbstractSocket::SocketState state);

private:
    QString host;
    qint16 port;
    bool ssl;
    QSslSocket *sslSocket;
};

