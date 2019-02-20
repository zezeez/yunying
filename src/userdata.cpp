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
    }
    return true;
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
    wxml.writeEndElement();
    return true;
}

UserData::~UserData()
{
    delete staMap;
    delete staCode;
}
