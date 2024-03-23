#ifdef WIN32
#include <WinSock2.h>
#endif
#include "lib/smtp/include/smtp.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string.h>

#define _ QStringLiteral


template<typename T>
std::string join(T& vecData, const std::string& delim)
{
    if (vecData.size() <= 0)
    {
        return std::string();
    }
    std::stringstream ss;
    for (auto& item : vecData)
    {
        ss << delim  << item ;
    }

    return ss.str().substr(delim.length());
}

const char MimeTypes[][2][128] =
{
    { "***",    "application/octet-stream" },
    { "csv",    "text/csv" },
    { "tsv",    "text/tab-separated-values" },
    { "tab",    "text/tab-separated-values" },
    { "html",    "text/html" },
    { "htm",    "text/html" },
    { "doc",    "application/msword" },
    { "docx",    "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
    { "ods",    "application/x-vnd.oasis.opendocument.spreadsheet" },
    { "odt",    "application/vnd.oasis.opendocument.text" },
    { "rtf",    "application/rtf" },
    { "sxw",    "application/vnd.sun.xml.writer" },
    { "txt",    "text/plain" },
    { "xls",    "application/vnd.ms-excel" },
    { "xlsx",    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
    { "pdf",    "application/pdf" },
    { "ppt",    "application/vnd.ms-powerpoint" },
    { "pps",    "application/vnd.ms-powerpoint" },
    { "pptx",    "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
    { "wmf",    "image/x-wmf" },
    { "atom",    "application/atom+xml" },
    { "xml",    "application/xml" },
    { "json",    "application/json" },
    { "js",    "application/javascript" },
    { "ogg",    "application/ogg" },
    { "ps",    "application/postscript" },
    { "woff",    "application/x-woff" },
    { "xhtml","application/xhtml+xml" },
    { "xht",    "application/xhtml+xml" },
    { "zip",    "application/zip" },
    { "gz",    "application/x-gzip" },
    { "rar",    "application/rar" },
    { "rm",    "application/vnd.rn-realmedia" },
    { "rmvb",    "application/vnd.rn-realmedia-vbr" },
    { "swf",    "application/x-shockwave-flash" },
    { "au",        "audio/basic" },
    { "snd",    "audio/basic" },
    { "mid",    "audio/mid" },
    { "rmi",        "audio/mid" },
    { "mp3",    "audio/mpeg" },
    { "aif",    "audio/x-aiff" },
    { "aifc",    "audio/x-aiff" },
    { "aiff",    "audio/x-aiff" },
    { "m3u",    "audio/x-mpegurl" },
    { "ra",    "audio/vnd.rn-realaudio" },
    { "ram",    "audio/vnd.rn-realaudio" },
    { "wav",    "audio/x-wave" },
    { "wma",    "audio/x-ms-wma" },
    { "m4a",    "audio/x-m4a" },
    { "bmp",    "image/bmp" },
    { "gif",    "image/gif" },
    { "jpe",    "image/jpeg" },
    { "jpeg",    "image/jpeg" },
    { "jpg",    "image/jpeg" },
    { "jfif",    "image/jpeg" },
    { "png",    "image/png" },
    { "svg",    "image/svg+xml" },
    { "tif",    "image/tiff" },
    { "tiff",    "image/tiff" },
    { "ico",    "image/vnd.microsoft.icon" },
    { "css",    "text/css" },
    { "bas",    "text/plain" },
    { "c",        "text/plain" },
    { "h",        "text/plain" },
    { "rtx",    "text/richtext" },
    { "mp2",    "video/mpeg" },
    { "mpa",    "video/mpeg" },
    { "mpe",    "video/mpeg" },
    { "mpeg",    "video/mpeg" },
    { "mpg",    "video/mpeg" },
    { "mpv2",    "video/mpeg" },
    { "mov",    "video/quicktime" },
    { "qt",    "video/quicktime" },
    { "lsf",    "video/x-la-asf" },
    { "lsx",    "video/x-la-asf" },
    { "asf",    "video/x-ms-asf" },
    { "asr",    "video/x-ms-asf" },
    { "asx",    "video/x-ms-asf" },
    { "avi",    "video/x-msvideo" },
    { "3gp",    "video/3gpp" },
    { "3gpp",    "video/3gpp" },
    { "3g2",    "video/3gpp2" },
    { "movie","video/x-sgi-movie" },
    { "mp4",    "video/mp4" },
    { "wmv",    "video/x-ms-wmv" },
    { "webm","video/webm" },
    { "m4v",    "video/x-m4v" },
    { "flv",    "video/x-flv" }
};


std::string fileBasename(const std::string path) 
{
    std::string filename = path.substr(path.find_last_of("/\\") + 1);//解析出文件名字
    return filename;
}

std::string getFileContents(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    else {
        printf("文件读取失败:%s\n",filename);
        return "";
    }
}
//获取文件的后缀名 如xxx.jpg 获取的是jpg
std::string GetFileExtension(const std::string& FileName)
{
    if (FileName.find_last_of(".") != std::string::npos)        //find_last_of逆向查找在原字符串中最后一个与指定字符串（或字符）中的某个字符匹配的字符，返回它的位置。若查找失败，则返回npos。
        return FileName.substr(FileName.find_last_of(".") + 1);
    return "";
}

const char* GetMimeTypeFromFileName(char* szFileExt)
{
    for (unsigned int i = 0; i < sizeof(MimeTypes) / sizeof(MimeTypes[0]); i++)
    {
        if (strcmp(MimeTypes[i][0], szFileExt) == 0)
        {
            return MimeTypes[i][1];
        }
    }
    return MimeTypes[0][1];   //if does not match any,  "application/octet-stream" is returned
}

std::string base64Encode(char const* origSigned, unsigned origLength)
{
    const char base64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char const* orig = (unsigned char const*)origSigned; // in case any input bytes have the MSB set
    if (orig == NULL) return "";

    unsigned const numOrig24BitValues = origLength / 3;
    bool havePadding = origLength > numOrig24BitValues * 3;
    bool havePadding2 = origLength == numOrig24BitValues * 3 + 2;
    unsigned const numResultBytes = 4 * (numOrig24BitValues + havePadding);
    char* result = new char[numResultBytes + 3]; // allow for trailing '/0'

    // Map each full group of 3 input bytes into 4 output base-64 characters:
    unsigned i;
    for (i = 0; i < numOrig24BitValues; ++i)
    {
        result[4 * i + 0] = base64Char[(orig[3 * i] >> 2) & 0x3F];
        result[4 * i + 1] = base64Char[(((orig[3 * i] & 0x3) << 4) | (orig[3 * i + 1] >> 4)) & 0x3F];
        result[4 * i + 2] = base64Char[((orig[3 * i + 1] << 2) | (orig[3 * i + 2] >> 6)) & 0x3F];
        result[4 * i + 3] = base64Char[orig[3 * i + 2] & 0x3F];
    }

    // Now, take padding into account.  (Note: i == numOrig24BitValues)
    if (havePadding)
    {
        result[4 * i + 0] = base64Char[(orig[3 * i] >> 2) & 0x3F];
        if (havePadding2)
        {
            result[4 * i + 1] = base64Char[(((orig[3 * i] & 0x3) << 4) | (orig[3 * i + 1] >> 4)) & 0x3F];
            result[4 * i + 2] = base64Char[(orig[3 * i + 1] << 2) & 0x3F];
        }
        else
        {
            result[4 * i + 1] = base64Char[((orig[3 * i] & 0x3) << 4) & 0x3F];
            result[4 * i + 2] = '=';
        }
        result[4 * i + 3] = '=';
    }

    result[numResultBytes] = '\0';
    std::string s(result);
    delete [] result;
    return s;
}

std::string SmtpEmail::getEmailBody()
{
    //设定邮件的发送者名称、接收者名称、邮件主题，邮件内容。
    std::ostringstream message;
    if (emailQueue.empty())
        return "";
    struct EmailInfo info = emailQueue.front();

    std::string encode = base64Encode(info.sender.c_str(), info.sender.length());
    message << "From: =?" << info.charset << "?b?" << encode << "?= <" << info.senderEmail << ">\r\n";

    std::vector<std::string> vecToList;
    for (auto &item : info.recvList)
    {
        encode = base64Encode(item.c_str(), item.length());
        std::string to = "=?" + info.charset + "?b?" + encode + "?= <" + item + ">";
        vecToList.push_back(to);
    }

    message << "To: " << join(vecToList, ",") << "\r\n";
    encode = base64Encode(info.subject.c_str(), info.subject.length());
    message << "Subject: =?" << info.charset << "?b?" << encode << "?=\r\n";
    message << "MIME-Version: 1.0\r\n";

    if (info.ccEmail.size() > 0)
    {
        std::vector<std::string> vecCcList;
        for (auto &item : info.ccEmail)
        {
            encode = base64Encode(item.c_str(), item.length());
            std::string cc = "=?" + info.charset + "?b?" + encode + "?= <" + item + ">";
            vecCcList.push_back(cc);
        }
        message << "Cc:" << join(vecCcList, ",") << "\r\n";
    }

    message << "Content-Type:multipart/mixed; boundary=\"Separator_ztq_000\"\r\n\r\n";
    message << "--Separator_ztq_000\r\n";
    message << "Content-Type: multipart/alternative; boundary=\"Separator_ztq_111\"\r\n\r\n";
    message << "--Separator_ztq_111\r\n";
    message << "Content-Type: " << "text/html" << "; charset=\"" << info.charset << "\"\r\n";
    message << "Content-Transfer-Encoding: base64\r\n";
    message << "\r\n";                  //此处要加，不然邮件正文无内容
    encode = base64Encode(info.message.c_str(), info.message.length());
    message << encode;
    message << "\r\n\r\n";
    message << "--Separator_ztq_111--\r\n";
    //---------------------文件部分处理--------------------------------------

    for (auto &item : info.attachment)
    {
        std::string filename = fileBasename(item);
        std::string strContext = getFileContents(item.c_str());
        if(strContext.empty())
        {
            std::cerr << "请检查传入的文件路径是否正确,此路径文件添加到附件失败，不发送此文件:" << std::endl;
            std::cerr << item << std::endl;
        }
        else
        {
            std::string fileContext = base64Encode(strContext.c_str(), strContext.length());
            std::string extension = GetFileExtension(filename);
            std::string mimetype = GetMimeTypeFromFileName((char*)extension.c_str());
            message << "--Separator_ztq_000\r\n";
            message << "Content-Type: " << mimetype << "; name=\"" << filename << "\"\r\n";
            message << "Content-Transfer-Encoding: base64\r\n";
            message << "Content-Disposition: attachment; filename=\"" << filename << "\"\r\n\r\n";
            message << fileContext + "\r\n\r\n";        //把读取到的文件内容以二进制形式发送
        }
    }
    //-----------------------------------------------------------
    message << "\r\n.\r\n";
    return message.str();
}

SmtpEmail::SmtpEmail(QObject *parent) : SmtpBase(parent)
{
    status = EHELO;
    isSuccess = false;
    // see RFC 5321 for error code definition
    okCode.insert({"211", "System status, or system help reply"});
    okCode.insert({"214", "Help message (Information on how to use the receiver or the "
                          "meaning of a particular non-standard command; this reply is useful"
                          "only to the human user)"});
    okCode.insert({"220", "<domain> Service ready"});
    okCode.insert({"221", "<domain> Service closing transmission channel"});
    okCode.insert({"234", "Authentication successful"});
    okCode.insert({"235", "Authentication successful"});
    okCode.insert({"250", "Requested mail action okay, completed"});
    okCode.insert({"251", "User not local; will forward to <forward-path>"});
    okCode.insert({"252", "Cannot VRFY user, but will accept message and attempt delivery"});
    okCode.insert({"334", "more data"});
    okCode.insert({"354", "Start mail input; end with <CRLF>.<CRLF>"});
    errCode.insert({"421", "<domain> Service not available, closing transmission channel"
                           "(This may be a reply to any command if the service knows it must shut down)"});
    errCode.insert({"450", "Requested mail action not taken: mailbox unavailable (e.g.,"
                           "mailbox busy or temporarily blocked for policy reasons)"});
    errCode.insert({"451", "Requested action aborted: local error in processing"});
    errCode.insert({"452", "Requested action not taken: insufficient system storage"});
    errCode.insert({"455", "Server unable to accommodate parameters"});
    errCode.insert({"500", "Syntax error, command unrecognized (This may include errors such as command line too long)"});
    errCode.insert({"501", "Syntax error in parameters or arguments"});
    errCode.insert({"502", "Command not implemented"});
    errCode.insert({"503", "Bad sequence of commands"});

    errCode.insert({"504", "Command parameter not implemented"});
    errCode.insert({"535", "Login Fail. Please enter your authorization code to login."});
    errCode.insert({"550", "Requested action not taken: mailbox unavailable (e.g., mailbox "
                               "not found, no access, or command rejected for policy reasons)"});
    errCode.insert({"551", "User not local; please try <forward-path>"});
    errCode.insert({"552", "Requested mail action aborted: exceeded storage allocation"});
    errCode.insert({"553", "Requested action not taken: mailbox name not allowed (e.g.,"
                                           "mailbox syntax incorrect)"});
    errCode.insert({"554", "Transaction failed (Or, in the case of a connection-opening "
                                               "response, \"No SMTP service here\")"});
    errCode.insert({"555", "MAIL FROM/RCPT TO parameters not recognized or not implemented"});

    nextStatus.resize(EQUIT + 2);
    /*nextStatus[EINIT] = [this]() {
        // 向服务器发送ehlo
        enum SMTPSTATUS status = emailQueue.empty() ? EHELO : EAUTH;
        std::string command = emailQueue.empty() ? "" : "ehlo EmailService\r\n";
        return std::pair<enum SMTPSTATUS, std::string>(status, command);
    };*/
    nextStatus[EHELO] = [this]() {
        // 向服务器发送ehlo
        enum SMTPSTATUS status = emailQueue.empty() ? EHELO : ETOAUTH;
        std::string command = emailQueue.empty() ? "" : "ehlo EmailService\r\n";
        return std::pair<enum SMTPSTATUS, std::string>(status, command);
    };
    nextStatus[ETOAUTH] = [this]() {
        if (emailQueue.empty()) {
            return std::pair<enum SMTPSTATUS, std::string>(EHELO, "");
        }
        // 进行登录验证
        std::string command = "AUTH PLAIN ";
        std::string auth = '\0' + emailQueue.front().senderEmail + '\0' + emailQueue.front().password;
        command += base64Encode(auth.data(), auth.size());
        command += "\r\n";
        return std::pair<enum SMTPSTATUS, std::string>(EMAILFROM, command);
    };
    nextStatus[EMAILFROM] = [this]() {
        if (emailQueue.empty()) {
            return std::pair<enum SMTPSTATUS, std::string>(EEND, "");
        }
        curReceiver = 0;
        // 设置邮件发送者的邮箱地址
        std::string command = "mail FROM:<" + emailQueue.front().senderEmail + ">\r\n";
        return std::pair<enum SMTPSTATUS, std::string>(ERCPTO, command);
    };
    nextStatus[ERCPTO] = [this]() {
        if (emailQueue.empty()) {
            return std::pair<enum SMTPSTATUS, std::string>(EEND, "");
        }
        // 设置邮件接收者的邮箱地址
        int recvListSize = emailQueue.front().recvList.size();
        int total = recvListSize + emailQueue.front().ccEmail.size();

        std::string command;
        if (curReceiver < recvListSize) {
            command += emailQueue.front().recvList[curReceiver];
        } else {
            if (curReceiver - recvListSize <
                (int)emailQueue.front().ccEmail.size()) {
                command += emailQueue.front().ccEmail[curReceiver - recvListSize];
            }
        }
        enum SMTPSTATUS status = ++curReceiver >= total ? EDATA : ERCPTO;
        return std::pair<enum SMTPSTATUS, std::string>(status, !command.empty() ?
                                                                   "RCPT TO:<" + command + ">\r\n" : "");
    };
    nextStatus[EDATA] = []() {
        std::string command = "data\r\n";
        return std::pair<enum SMTPSTATUS, std::string>(EMAILBODY, command);
    };
    nextStatus[EMAILBODY] = [this]() {
        // 发送邮件正文
        std::string command = getEmailBody();
        return std::pair<enum SMTPSTATUS, std::string>(EEND, command);
    };
    nextStatus[EEND] = [this]() {
        if (!emailQueue.empty()) {
            emailQueue.pop();
            isSuccess  = true;
        }
        //enum SMTPSTATUS status = emailQueue.empty() ? EINIT : EHELO;
        enum SMTPSTATUS status = EHELO;
        std::string command = emailQueue.empty() ? "QUIT\r\n" : "ehlo EmailService\r\n";
        return std::pair<enum SMTPSTATUS, std::string>(status, command);
    };
    nextStatus[ERESET] = []() {
        // 发生错误需要重置
        enum SMTPSTATUS status = EQUIT;
        std::string command = "RSET\r\n";
        return std::pair<enum SMTPSTATUS, std::string>(status, command);
    };
    nextStatus[EQUIT] = []() {
        // 退出
        enum SMTPSTATUS status = EHELO;
        std::string command = "QUIT\r\n";
        return std::pair<enum SMTPSTATUS, std::string>(status, command);
    };
}

SmtpEmail::~SmtpEmail()
{

}

int SmtpEmail::sendEmail(const std::string& from, const std::string& passwd, const std::string& to, const std::string& subject, const std::string& strMessage)
{
    std::vector<std::string> vecTo;
    vecTo.push_back(to);
    return sendEmail(from, passwd, vecTo, subject, strMessage, std::vector<std::string>(), std::vector<std::string>());
}

int SmtpEmail::sendEmail(const std::string& from, const std::string& passwd, const std::vector<std::string>& vecTo,
                         const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment, const std::vector<std::string>& ccList)
{
    struct EmailInfo info;
    info.charset = "UTF-8";
    info.sender = from;
    info.password = passwd;
    info.senderEmail = from;;
    //info.recipientEmail = item;
    //info.recvList = std::move(vecTo);
    info.recvList = vecTo;
    info.subject = subject;
    info.message = strMessage;
    info.attachment = attachment;
    //info.ccEmail = std::move(ccList);
    info.ccEmail = ccList;
    emailQueue.push(info);

    return 0;
}

std::pair<enum SMTPSTATUS, std::string> SmtpEmail::smtpProtocolRun(const std::string &input, std::string &error)
{
    std::pair<enum SMTPSTATUS, std::string> ret;
    if (input.length() < 3) {
        ret = nextStatus[EQUIT]();
        status = ret.first;
        return ret;
    }
    std::string code = input.substr(0, 3);
    std::map<std::string, std::string>::iterator errIt;
    std::map<std::string, std::string>::iterator okIt;

    okIt = okCode.find(code);
    if (okIt != okCode.end()) {
        ret = nextStatus[status]();
    } else {
        errIt = errCode.find(code);
        if (errIt != errCode.end()) {
            error = errIt->second;
        } else {
            error = "Unknown error";
        }
        ret = nextStatus[EQUIT]();
    }
    enum SMTPSTATUS lastStatus = status;
    status = ret.first;
    ret.first = lastStatus;
    return ret;
}

bool SmtpEmail::mailSuccess()
{
    bool success = isSuccess;
    isSuccess = false;
    return success;
}

SimpleSmtpEmail::SimpleSmtpEmail(QObject *parent) :
    SmtpEmail(parent)
{
    sslSocket = new QSslSocket;
    connect(sslSocket, &QSslSocket::stateChanged,
            this, &SimpleSmtpEmail::socketStateChanged);
    connect(sslSocket, &QSslSocket::encrypted,
            this, &SimpleSmtpEmail::socketEncrypted);
    connect(sslSocket, &QSslSocket::errorOccurred,
            this, &SimpleSmtpEmail::socketError);
    connect(sslSocket, QOverload<const QList<QSslError> &>::of(&QSslSocket::sslErrors),
            this, &SimpleSmtpEmail::sslError);
    connect(sslSocket, &QSslSocket::readyRead,
            this, &SimpleSmtpEmail::readyRead);
}

SimpleSmtpEmail::~SimpleSmtpEmail()
{
    sslSocket->close();
    delete sslSocket;
}

int SimpleSmtpEmail::sendEmail(const std::string& from, const std::string& passwd, const std::string& to,
                               const std::string& subject, const std::string& strMessage)
{
    SmtpEmail::sendEmail(from, passwd, to, subject, strMessage);
    if (!host.isEmpty() && port) {
        connectToHost();
    }
    return 0;
}

int SimpleSmtpEmail::sendEmail(const std::string& from, const std::string& passwd,
                               const std::vector<std::string>& vecTo,
                               const std::string& subject, const std::string& strMessage,
                               const std::vector<std::string>& attachment,
                               const std::vector<std::string>& ccList)
{
    SmtpEmail::sendEmail(from, passwd, vecTo, subject, strMessage, attachment, ccList);
    if (!host.isEmpty() && port) {
        connectToHost();
    }
    return 0;
}

void SimpleSmtpEmail::connectToHost()
{
    if (ssl) {
        sslSocket->connectToHostEncrypted(host, port);
    } else {
        sslSocket->connectToHost(host, port);
    }
}

void SimpleSmtpEmail::setHost(const QString &host)
{
    this->host = host;
}

void SimpleSmtpEmail::setPort(qint16 port)
{
    this->port = port;
}

void SimpleSmtpEmail::setSsl(bool ssl)
{
    this->ssl = ssl;
}

void SimpleSmtpEmail::disconnect()
{
    sslSocket->close();
}

void SimpleSmtpEmail::readyRead()
{
    std::pair<enum SMTPSTATUS, std::string> status;
    std::string error;
    QByteArray data = sslSocket->readAll();
    status = smtpProtocolRun(data.toStdString(), error);

    qDebug() << data;
    if (!status.second.empty()) {
        sslSocket->write(QByteArray::fromStdString(status.second));
    }

    if (!error.empty()) {
        int errIndex = data.indexOf("\r\n");
        QByteArray errDetail;
        if (errIndex > 4) {
            errDetail = data.sliced(4, errIndex - 4);
        }
        emit mailStateNotify(EMAILFAILED, QString::fromStdString(error),
                             _("邮件发送失败，错误详情: %1, 错误描述：%2")
                                 .arg(errDetail, QString::fromStdString(error)));
    } else {
        if (status.first == EEND) {
            bool success = mailSuccess();
            if (success) {
                emit mailStateNotify(EMAILSUCCESS, "", "");
            }
        }
    }
}

void SimpleSmtpEmail::socketError()
{

}

void SimpleSmtpEmail::sslError()
{

}

void SimpleSmtpEmail::socketEncrypted()
{

}

void SimpleSmtpEmail::socketStateChanged(QAbstractSocket::SocketState state)
{

}

void test()
{
    std::string from = "gxzpljj@163.com";
    std::string passwd ="YJXGIPMJMLWRGYMB";//这里替换成自己的授权码
    //std::string to = "953127304@qq.com";
    std::string subject = "test message";
    std::string strMessage = "<!DOCTYPE html><html><head><title>Page Title</title></head><body><h1>This is a Heading</h1><p>This is a paragraph.</p></body></html>";

    std::vector<std::string> vecTo; //发送列表
    vecTo.push_back("gxzpljj@163.com");

    std::vector<std::string> ccList;
    //ccList.push_back("953127304@qq.com");//抄送列表

    std::vector<std::string> attachment;
    //attachment.push_back("/home/drliu/Downloads/Ant.tar");

    static SimpleSmtpEmail m_mail;
    //base = &m_mail;
    //base->SendEmail(from, passs, to, subject, strMessage);//普通的文本发送，明文发送

    static SimpleSmtpEmail sslEmail;
    sslEmail.setHost("smtp.163.com");
    sslEmail.setPort(25);
    //sslEmail.setSsl(true);

    //base->SendEmail(from, passs, to, subject, strMessage);
    sslEmail.sendEmail(from, passwd,vecTo,subject,strMessage,attachment,ccList);//加密的发送，支持抄送、附件等
}
