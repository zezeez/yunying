#ifndef USERDATA_H
#define USERDATA_H
#include <QHash>
#include <QMultiHash>
#include <QStringList>
#include <QMap>
#include <QIODevice>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QString>
#include <QVariantMap>


enum ESEATTYPEENUM {
    ESEATSPECIALSEAT = 0,
    ESEATFIRSTPRISEAT,
    ESEATSECONDPRISEAT,
    ESEATADVSOFTCROUCH,
    ESEATSOFTCROUCH,
    ESEATSTIRCROUCH,
    ESEATHARDCROUCH,
    ESEATSOFTSEAT,
    ESEATHARDSEAT,
    ESEATNOSEAT,
    ESEATTYPEMAX
};

enum TrainTableColumnEnum {
    ETRAINNOCOL = 0,
    EFROMSTATIONCOL,
    ETOSTATIONCOL,
    ESTARTTIMECOL,
    EARRIVETIMECOL,
    EUSEDTIMECOL,
    ESPECIALSEATCOL,
    EFIRSTPRISEATCOL,
    ESECONDPRISEATCOL,
    EADVSOFTCROUCHCOL,
    ESOFTCROUCHCOL,
    ESTIRCROUCHCOL,
    EHARDCROUCHCOL,
    ESOFTSEATCOL,
    EHARDSEATCOL,
    ENOSEATCOL,
    EOTHERCOL,
    EREMARKCOL,
    ETRAINTABLECOLUMNENDTOTAL
};

static QStringList trainTableColumnDesc = {
    "车次",
    "出发站",
    "到达站",
    "出发时间",
    "到达时间",
    "用时",
    "商务/特等",
    "一等座",
    "二等座",
    "高级软卧",
    "软卧",
    "动卧",
    "硬卧",
    "软座",
    "硬座",
    "无座",
    "其他",
    "备注"
};

enum TrainInfoEnum {
    ESECRETSTR = 0,
    ETEXTINFO,
    ETRAINNO,
    ESTATIONTRAINCODE,
    ESTARTSTATIONTELECODE,
    EENDSTATIONTELECODE,
    EFROMSTATIONTELECODE,
    ETOSTATIONTELECODE,
    ESTARTTIME,
    EARRIVETIME,
    ESPENDTIME,
    ECANWEBBUY,
    EYPINFO,
    ESTARTTRAINDATE,
    ETRAINSEATFEATURE,
    ELOCATIONCODE,
    EFROMSTATIONNO,
    ETOSTATIONNO,
    EISSUPPORTCARD,
    ECOTROLLEDTRAINFLAG,
    EGGNUM,
    EGRNUM,  // 高级软卧
    EQTNUM,  // 其他
    ERWNUM,  // 软卧
    ERZNUM,  // 软座
    ETZNUM,  //
    EWZNUM,  // 无座
    EYBNUM,  // 动卧
    EYWNUM,  // 硬卧
    EYZNUM,  // 硬座
    EZENUM,  // 二等座
    EZYNUM,  // 一等座
    ESWZNUM,  // 商务座
    ESRBNUM,  //
    EYPEX,
    ESEATTYPES,
    EEXCHANGETRAINFLAG,
    ECANDIDATETRAINFLAG,
    ECANDIDATESEATLIMIT,
    EYPINFONEW,
    EDWFLAG = 46,
    ESTOPCHECKTIME = 48,
    ECOUNTRYFLAG,
    ELOCALARRIVETIME,
    ELOCALSTARTTIME,
    EBEDLEVELINFO = 53,
    ESEATDISCOUNTINFO,
    ESALETIME,
};

enum SHOWTRAINTYPE {
    EALL,
    EGTRAIN,
    EDTRAIN,
    ECTRAIN,
    EZTRAIN,
    ETTRAIN,
    EKTRAIN,
    EOTRAIN
};

enum RUNSTATUSE {
    EINIT,
    EIDLE,
    EGRABTICKET,
    ESUBMITORDER,
    ESUBMITORDERSUCCESS,
    EGRABCANDIDATETICKET,
    ESUBMITCANDIDATE,
    EWAITDIFFDATEDATA,
    EGETDIFFDATEDATAFAILED,
    ESUBMITCANDIDATESUCCESS
};

enum GRABTICKETMODEE {
    ESHORTINTERVAL,
    ELONGINTERVAL,
    ESHORTANDLONG,
    ERANDOM,
    EFIXEDTIME,
    ECUSTOM
};

struct UserConfig {
    QString staFromName;
    QString staToName;
    QString staFromCode;
    QString staToCode;
    QString tourDate;
    bool showTicketPrice;
};

#define MAX_ALPHA_NUM 26

struct PassengerInfo {
    inline bool operator == (const struct PassengerInfo &info) const {
        if (!passName.compare(info.passName) && !passIdNo.compare(info.passIdNo))
            return true;
        return false;
    }
    QString allEncStr;
    QString passName;
    QString code;
    QString passIdTypeName;
    QString passIdTypeCode;
    QString passIdNo;
    QString passType;
    QString passTypeName;
    QString mobile;
    QString phone;
    QString indexId;
};

struct UserLoginInfo {
    QString account;
    QString passwd;
    QString userName;
};

struct SubmitTicketInfo {
    QString trainCode;
    QString trainNo;
    QString secretStr;
    QString ypDetailInfo;
    QString date;
    QString fromStationName;
    QString fromStationCode;
    QString toStationName;
    QString toStationCode;
    QVector<QChar> submitSeatType;
    bool isAsync;
    bool submitSuccess;
    QString passengerTicketInfo;
    QString oldPassengerTicketInfo;
    QString repeatSubmitToken;
    QString leftTicketStr;
    QString purposeCodes;
    QString trainLocation;
    QString keyCheckIsChange;
};

struct SeatTypePrioSetting {
    bool strictSeatTypePrio;
    bool sufficientSeatTypePrio;
    bool priceLowSeatTypePrio;
    bool priceHighSeatTypePrio;
    bool priceLowSeatTypePrioWhensufficient;
};

struct TimeRange {
    int beginHour;
    int beginMinute;
    int endHour;
    int endMinute;
};

struct TrainPrioSetting {
    bool strictTrainPrio;
    bool strictStartTimePrio;
    bool sufficientTicketPrio;
    bool travelTimeShortPrio;
    bool preferGPrio;
    bool preferDPrio;
    bool preferTimeRangePrio;
    struct TimeRange timeRange;
};

struct TicketSetting {
    bool partialSubmit;
};

struct CandidateSetting {
    QString secretList;
    int selectedEndCandidateMinutes;
    int endCandidateMinutes;
    bool isCandidate;
    bool prioCandidate;
    bool onlyCandidate;
    bool acceptNoSeat;
    bool acceptNewTrain;
    bool extraCandidateDateEn;
    bool extraCandidateDate2En;
    QString extraCandidateDate;
    QString extraCandidateDate2;
    int extraCandidateStartHour;
    int extraCandidateEndHour;
};

struct EmailNofity {
    bool notifyEmailEnable;
    QString senderEmailAddress;
    QString authCode;
    QVector<QString> receiverEmailAddress;
    QString emailServer;
    quint16 emailPort;
    bool enableSsl;
    bool keepAuthCode;
    bool advanceMode;
};

struct NotifySetting {
    struct EmailNofity emailNotify;
};

struct GrabTicketSetting {
    QVector<struct PassengerInfo> selectedPassenger;  //  选中的旅客信息
    QVector<struct PassengerInfo> selectedDjPassenger;  // 选中的受让人旅客信息
    QVector<QString> trainNo;
    QVector<QChar> seatTypeCode;
    struct SeatTypePrioSetting seatTypePrio;
    struct TrainPrioSetting trainPrio;
    struct TicketSetting ticketSetting;
    bool autoFrozenTrain;
    int frozenSeconds;
    int grabTicketDate;
    int grabTicketHour;
    int grabTicketMinute;
    int grabTicketSecond;
    enum GRABTICKETMODEE grabMode;
    int grabIntervalSeconds;
};

struct GeneralSetting {
    bool autoSyncServerTime;
};

class UserData
{
public:
    ~UserData();
    void setStationCode(const QByteArray &staName, const QByteArray &staCode);

    static inline UserData *instance()
    {
        static UserData ud;
        return &ud;
    }

    inline struct UserConfig &getUserConfig()
    {
        return userConfig;
    }
    inline struct UserLoginInfo &getUserLoginInfo()
    {
        return loginInfo;
    }
    inline void setUserLoginName(QString name)
    {
        loginInfo.userName = name;
    }
    inline void setUserLoginPaswd(QString passwd)
    {
        loginInfo.passwd = passwd;
    }
    inline struct GrabTicketSetting &getGrabTicketSetting()
    {
        return grabSetting;
    }
    inline QHash<QString, QString> *getStaCode()
    {
        return staCode;
    }
    QString getApptk()
    {
        return apptk;
    }
    void setApptk(QString tk)
    {
        apptk = tk;
    }
    QString getUamtk()
    {
        return uamtk;
    }
    void setUamtk(QString tk)
    {
        uamtk = tk;
    }
    QString seatTypeToDesc(int idx);
    enum ESEATTYPEENUM SeatDescToType(QString desc);

    struct PassengerInfo setPassengerInfo(QVariantMap &map);
    bool whatsSelect(bool onlyNormal);
    const QString getpassengerTickets();
    bool isTimeInRange(int hour, int minute);

    bool readConfigFile();
    bool readConfig();
    bool writeConfigFile();
    bool writeConfig();
    void setRunStatus(enum RUNSTATUSE status) {
        lastRunStatus = runStatus;
        runStatus = status;
    }
    void recoverRunStatus() {
        runStatus = lastRunStatus;
    }

    QString errorString() const;
private:
    explicit UserData();
    UserData(const UserData &) Q_DECL_EQ_DELETE;
    UserData &operator=(const UserData &rhs) Q_DECL_EQ_DELETE;
public:
    QHash<QString, QString> *staCode;
    struct UserConfig userConfig;
    struct UserLoginInfo loginInfo;

    struct GrabTicketSetting grabSetting;
    struct GeneralSetting generalSetting;
    struct SubmitTicketInfo submitTicketInfo;
    struct CandidateSetting candidateSetting;
    struct NotifySetting notifySetting;
    QVector<struct PassengerInfo> passenger;  // 旅客信息
    QVector<struct PassengerInfo> djPassenger;  //  受让人旅客信息
    QString apptk;
    QString uamtk;
    enum RUNSTATUSE runStatus;
    enum RUNSTATUSE lastRunStatus;
    enum RUNSTATUSE candidateRunStatus;

    void readXBEL();
    QXmlStreamReader rxml;
    QXmlStreamWriter wxml;
};

QString seatTypeSubmtiCodeTransToDesc(QChar seatType);

#endif // USERDATA_H
