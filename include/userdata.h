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

struct UserConfig {
    QString staFromName;
    QString staToName;
    QString staFromCode;
    QString staToCode;
    QString tourDate;
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
    inline QMap<QString, QStringList> *getStaMap()
    {
        return staMap;
    }
    inline QHash<QString, QString> *getStaCode()
    {
        return staCode;
    }

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

    void readXBEL();
    QXmlStreamReader rxml;
    QXmlStreamWriter wxml;
};

#endif // USERDATA_H
