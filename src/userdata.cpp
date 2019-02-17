#include "userdata.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

UserData::UserData(): staMap(new QMap<QString, QStringList>),
    staCode(new QHash<QString, QString>)
{
    QFile file(QString("/home/drawer/12306ticket/station_name.txt"));

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream data(&file);
        QString text = data.readAll();
        QStringList staInfo = text.split('@');
        QStringList::const_iterator it;

        for (it = staInfo.cbegin(); it != staInfo.cend(); ++it) {
            QStringList staName = it->split('|');
            staMap->insertMulti(staName[EFULLSTAPINYIN], staName);
            staMap->insertMulti(staName[ESIMPLESTAPINYIN], staName);
            staCode->insert(staName[ESTANAME], staName[ESTACODE]);
        }
        file.close();
    }
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
