#include "userdata.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTextStream>
#include "nethelper.h"

UserData::UserData(): staMap(new QMap<QString, QStringList>),
    staCode(new QHash<QString, QString>)
{
    if (readStationFile("./station_info.txt") < 0) {
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
        staMap->insertMulti(staName[EFULLSTAPINYIN], staName);
        staMap->insertMulti(staName[ESIMPLESTAPINYIN], staName);
        staCode->insert(staName[ESTANAME], staName[ESTACODE]);
    }
}

int UserData::writeStationFile(const QByteArray &data)
{
    QFile file(QStringLiteral("./station_info.txt"));

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

UserData::~UserData()
{
    delete staMap;
}
