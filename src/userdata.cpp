#include "userdata.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "nethelper.h"

UserData::UserData(): staMap(new QMap<QString, QStringList>),
    staCode(new QHash<QString, QString>)
{
    if (readStationFile("./station_name.txt") < 0) {
        NetHelper::instance()->getStationNameTxt();
    }
}

int UserData::readStationFile(const QString &filename)
{
    QFile file(filename);

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream data(&file);
        QString text = data.readAll();
        setStationInfo(text);
        file.close();
        return 0;
    }
    return -1;
}

void UserData::setStationInfo(const QString &data)
{
    QStringList staInfo = data.split('@');
    QStringList::const_iterator it;

    for (it = staInfo.cbegin(), ++it; it != staInfo.cend(); ++it) {
        QStringList staName = it->split('|');
        if (staName.size() >=  ESTACURRENTCOUNT) {
            staMap->insertMulti(staName[EFULLSTAPINYIN], staName);
            staMap->insertMulti(staName[ESIMPLESTAPINYIN], staName);
            staCode->insert(staName[ESTANAME], staName[ESTACODE]);
        }
    }
}

int UserData::writeStationFile(const QByteArray &data)
{
    QFile file(QStringLiteral("./station_name.txt"));

    setStationInfo(data);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << data;
        file.close();
        return 0;
    }
    return -1;
}

void UserData::proccessStationNameTxt(const QByteArray &data)
{
    writeStationFile(data);
}

UserData *UserData::instance()
{
    static UserData ud;
    return &ud;
}

bool UserData::readConfigFile()
{
    QFile file(QStringLiteral("./config.xml"));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    rxml.setDevice(&file);

    if (rxml.readNextStartElement()) {
        readConfig();
    }

    return !rxml.error();
}

bool UserData::readConfig()
{
    Q_ASSERT(rxml.isStartElement() && rxml.name() == QLatin1String("config"));

    while (rxml.readNextStartElement()) {
        if (rxml.name() == QLatin1String("FromStationName"))
            userConfig.staFromName = rxml.readElementText();
        else if (rxml.name() == QLatin1String("ToStationName"))
            userConfig.staToName = rxml.readElementText();
        else if (rxml.name() == QLatin1String("TourDate"))
            userConfig.tourDate = rxml.readElementText();
        else if (rxml.name() == QLatin1String("Account"))
            detailInfo.account = rxml.readElementText();
        else if (rxml.name() == QLatin1String("Passwd"))
            detailInfo.passwd = rxml.readElementText();
        else {
            qDebug() << "Ignore unrecognise field " << rxml.name() << endl;
        }
    }
    return true;
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

enum ESEATTYPEENUM UserData::SeatDescToType(QString desc)
{
    if (!desc.compare(QStringLiteral("特等座")))
        return ESEATSPECIALSEAT;
    else if (!desc.compare(QStringLiteral("一等座")))
        return ESEATFIRSTPRISEAT;
    else if (!desc.compare(QStringLiteral("二等座")))
        return ESEATSECONDPRISEAT;
    else if (!desc.compare(QStringLiteral("高级软卧")))
        return ESEATADVSOFTCROUCH;
    else if (!desc.compare(QStringLiteral("软卧")))
        return ESEATSOFTCROUCH;
    else if (!desc.compare(QStringLiteral("动卧")))
        return ESEATSTIRCROUCH;
    else if (!desc.compare(QStringLiteral("硬卧")))
        return ESEATHARDCROUCH;
    else if (!desc.compare(QStringLiteral("软座")))
        return ESEATSOFTSEAT;
    else if (!desc.compare(QStringLiteral("硬座")))
        return ESEATHARDSEAT;
    else if (!desc.compare(QStringLiteral("无座")))
        return ESEATNOSEAT;
    else
        return ESEATTYPEINVALID;

}

bool UserData::writeConfigFile()
{
    QFile file(QStringLiteral("./config.xml"));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    wxml.setDevice(&file);

    wxml.writeStartDocument();
    //wxml.writeDTD(QStringLiteral("<!DOCTYPE xbel>"));
    //wxml.writeStartElement(QStringLiteral("xbel"));
    //wxml.writeAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
    writeConfig();

    wxml.writeEndDocument();
    return true;
}

bool UserData::writeConfig()
{
    wxml.writeStartElement(QStringLiteral("config"));
    //wxml.writeAttribute(QStringLiteral("station"))
    wxml.writeTextElement(QStringLiteral("FromStationName"), userConfig.staFromName);
    //wxml.writeTextElement(QStringLiteral("FromStationCode"), userConfig.staFromCode);
    wxml.writeTextElement(QStringLiteral("ToStationName"), userConfig.staToName);
    //wxml.writeTextElement(QStringLiteral("ToStationCode"), userConfig.staToCode);
    wxml.writeTextElement(QStringLiteral("TourDate"), userConfig.tourDate);
    wxml.writeTextElement(QStringLiteral("Account"), detailInfo.account);
    wxml.writeTextElement(QStringLiteral("Passwd"), detailInfo.passwd);
    wxml.writeEndElement();
    return true;
}

UserData::~UserData()
{
    delete staMap;
    delete staCode;
}
