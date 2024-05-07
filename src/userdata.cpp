#include "userdata.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

#define _ QStringLiteral

UserData::UserData(): staCode(new QHash<QString, QString>)
{
}

void UserData::setStationCode(const QByteArray &name, const QByteArray &code)
{
    staCode->insert(QString::fromUtf8(name), QString::fromUtf8(code));
}

QString UserData::seatTypeToDesc(int idx)
{
    switch (idx) {
    case ESEATSPECIALSEAT:
        return QStringLiteral("特等座");
    case ESEATFIRSTPRISEAT:
        return QStringLiteral("一等座");
    case ESEATSECONDPRISEAT:
        return QStringLiteral("二等座");
    case ESEATADVSOFTCROUCH:
        return QStringLiteral("高级软卧");
    case ESEATSOFTCROUCH:
        return QStringLiteral("软卧");
    case ESEATSTIRCROUCH:
        return QStringLiteral("动卧");
    case ESEATHARDCROUCH:
        return QStringLiteral("硬卧");
    case ESEATSOFTSEAT:
        return QStringLiteral("软座");
    case ESEATHARDSEAT:
        return QStringLiteral("硬座");
    case ESEATNOSEAT:
        return QStringLiteral("无座");
    default:
        return "";
    }
}

struct PassengerInfo UserData::setPassengerInfo(QVariantMap &map)
{
    QString idx;
    struct PassengerInfo pinfo;

    idx = map[QLatin1String("allEncStr")].toString();
    pinfo.allEncStr = idx;

    idx = map[QLatin1String("passenger_name")].toString();
    pinfo.passName = idx;

    idx = map[QLatin1String("passenger_id_type_code")].toString();
    pinfo.passIdTypeCode = idx;

    idx = map[QLatin1String("passenger_id_type_name")].toString();
    pinfo.passIdTypeName = idx;

    idx = map[QLatin1String("passenger_id_no")].toString();
    pinfo.passIdNo = idx;

    idx = map[QLatin1String("passenger_type")].toString();
    pinfo.passType = idx;

    idx = map[QLatin1String("passenger_type_name")].toString();
    pinfo.passTypeName = idx;

    idx = map[QLatin1String("mobile_no")].toString();
    pinfo.mobile = idx;

    idx = map[QLatin1String("phone_no")].toString();
    pinfo.phone = idx;

    idx = map[QLatin1String("index_id")].toString();
    pinfo.indexId = idx;

    return pinfo;
}

bool UserData::whatsSelect(bool onlyNormal)
{
    if (onlyNormal) {
        if (djPassenger.size() == 0)
            return true;
        return grabSetting.selectedPassenger.size() != 0;
    }
    return grabSetting.selectedDjPassenger.size() != 0;
}

bool UserData::isTimeInRange(int hour, int minute)
{
    int beginHour = grabSetting.trainPrio.timeRange.beginHour;
    int endHour = grabSetting.trainPrio.timeRange.endHour;
    if (hour >= beginHour && hour <= endHour) {
        int beginMinute = grabSetting.trainPrio.timeRange.beginMinute;
        int endMinute = grabSetting.trainPrio.timeRange.endMinute;
        if (hour == beginHour && hour == endHour) {
            return minute >= beginMinute && minute <= endMinute;
        } else if (hour == beginHour) {
            return minute >= beginMinute;
        } else if (hour == endHour) {
            return minute <= endMinute;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

UserData::~UserData()
{
    delete staCode;
}

QString seatTypeSubmtiCodeTransToDesc(QChar seatType)
{
    switch (seatType.toLatin1()) {
    case '1':
        return _("硬座");
    case '3':
        return _("硬卧");
    case '4':
        return _("软卧");
    case '9':
        return _("商务");
    case 'A':
        return _("高级动卧");
    case 'F':
        return _("动卧");
    case 'O':
        return _("二等座");
    case 'M':
        return _("一等座");
    case 'P':
        return _("特等座");
    case 'W':
        return _("无座");
    default:
        return _("");
    }
}

QChar seatTypeEnumTransToSubmitCode(enum TrainInfoEnum seatType)
{
    const QVector<QChar> code = {
        '0',
        '6',
        '0',
        '4',
        '2',
        'P',
        'W',
        '0',
        '3',
        '1',
        'O',
        'M',
        '9',
        'F',
    };
    int idx = seatType - EGGNUM;
    if (idx < 0 || idx >= code.size())
        return '0';
    return code[idx];
}
