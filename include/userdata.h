#ifndef USERDATA_H
#define USERDATA_H
#include <QHash>
#include <QMultiHash>
#include <QStringList>
#include <QMap>
#include <QIODevice>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

enum stationInfoEnum {
    ETHREEPINYINCODE = 0,
    ESTANAME,
    ESTACODE,
    EFULLSTAPINYIN,
    ESIMPLESTAPINYIN,
    ESTACURRENTCOUNT,
};

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
    ESEATTYPEINVALID = -1
};

enum TrainTableColumnEnum {
    ETRAINNUM = 0,
    EFROMTOSTATION,
    ESTARTARRIVETIME,
    EUSEDTIME,
    ESPECIALSEAT,
    EFIRSTPRISEAT,
    ESECONDPRISEAT,
    EADVSOFTCROUCH,
    ESOFTCROUCH,
    ESTIRCROUCH,
    EHARDCROUCH,
    ESOFTSEAT,
    EHARDSEAT,
    ENOSEAT,
    EOTHER,
    EREMARK,
    ETRAINTABLECOLUMNEND
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
    ESPEEDTIME,
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
    EGRNUM,
    EQTNUM,
    ERWNUM,
    ERZNUM,
    ETZNUM,
    EWZNUM,
    EYBNUM,
    EYWNUM,
    EYZNUM,
    EZENUM,
    EZYNUM,
    ESWZNUM,
    ESRBNUM,
    EYPEX,
    ESEATTYPES,
    EUNKNOWN1,
    EUNKNOWN2
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

struct UserConfig {
    QString staFromName;
    QString staToName;
    QString staFromCode;
    QString staToCode;
    QString tourDate;
};

struct PassengerInfo {
    inline bool operator == (const struct PassengerInfo &info) const {
        if (!passName.compare(info.passName) && !passIdNo.compare(info.passIdNo))
            return true;
        return false;
    }
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

struct UserDetailInfo {
    QString account;
    QString passwd;
    QString userName;
    QVector<struct PassengerInfo> passenger;
};

struct UserSetting {
    bool showTrain[8];
};

struct TrainInfo {
    QString securityStr;
    QString trainNo;
};

struct GrabTicketSetting {
    QVector<struct PassengerInfo> selectedPassenger;
    QVector<QString> trainNo;
    QVector<enum ESEATTYPEENUM> seatType;
};

class UserData
{
public:
    static UserData *instance();
    ~UserData();
    void proccessStationNameTxt(const QByteArray &date);
    int readStationFile(const QString &file);
    int writeStationFile(const QByteArray &data);
    void setStationInfo(const QString &data);

    inline struct UserConfig &getUserConfig()
    {
        return userConfig;
    }
    inline struct UserDetailInfo &getUserDetailInfo()
    {
        return detailInfo;
    }
    inline QVector<struct TrainInfo> &getTrainInfo()
    {
        return trainInfo;
    }
    inline struct UserSetting &getUserSetting()
    {
        return setting;
    }
    inline struct GrabTicketSetting &getGrabTicketSetting()
    {
        return grabSetting;
    }
    inline QMap<QString, QStringList> *getStaMap()
    {
        return staMap;
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
    int getTableViewIdx()
    {
        return tableViewIdx;
    }
    void setTableViewIdx(int idx)
    {
        tableViewIdx = idx;
    }
    QString seatTypeToDesc(int idx);
    enum ESEATTYPEENUM SeatDescToType(QString desc);

    bool readConfigFile();
    bool readConfig();
    bool writeConfigFile();
    bool writeConfig();

    QString errorString() const;
private:
    explicit UserData();
    UserData(const UserData &) Q_DECL_EQ_DELETE;
    UserData &operator=(const UserData &rhs) Q_DECL_EQ_DELETE;
private:
    QMap<QString, QStringList> *staMap;
    QHash<QString, QString> *staCode;
    struct UserConfig userConfig;
    struct UserDetailInfo detailInfo;
    struct UserSetting setting;
    struct GrabTicketSetting grabSetting;
    QVector<struct TrainInfo> trainInfo;
    QList<QString> wantedTrain;
    int tableViewIdx;
    QString apptk;
    QString uamtk;

    void readXBEL();
    QXmlStreamReader rxml;
    QXmlStreamWriter wxml;
};

#endif // USERDATA_H
