#ifndef USERDATA_H
#define USERDATA_H
#include <QHash>
#include <QMultiHash>
#include <QStringList>
#include <QMap>

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

class UserData
{
public:
    static UserData *instance();
    ~UserData();

private:
    explicit UserData();
    UserData(const UserData &) Q_DECL_EQ_DELETE;
    UserData &operator=(const UserData &rhs) Q_DECL_EQ_DELETE;
public:
    QMap<QString, QStringList> *staMap;
    QHash<QString, QString> *staCode;
    QString staFromCode, staToCode;
};

#endif // USERDATA_H
