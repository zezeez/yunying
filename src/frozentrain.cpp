#include "frozentrain.h"
#include <QDebug>
#include <QObject>

FrozenTrain::FrozenTrain(QObject *parent) : QObject(parent)
{
    frozenTimer.setInterval(1000);
    connect(&frozenTimer, &QTimer::timeout, this, &FrozenTrain::detectFrozenTimeout);
}

FrozenTrain::~FrozenTrain()
{

}

void FrozenTrain::addFrozenTrain(const QString &trainDesc, int seconds)
{
    frozenTrain.insert(trainDesc, seconds);
    if (!frozenTimer.isActive()) {
        frozenTimer.start();
    }
}

void FrozenTrain::removeFrozenTrain(const QString &trainDesc)
{
    frozenTrain.remove(trainDesc);
    if (frozenTrain.isEmpty() && frozenTimer.isActive()) {
        frozenTimer.stop();
    }
}

bool FrozenTrain::queryTrainIsFrozen(const QString &trainDesc)
{
    return frozenTrain.find(trainDesc) != frozenTrain.end();
}

void FrozenTrain::detectFrozenTimeout()
{
    QMap<QString, int>::iterator it;
    int remainSeconds;
    for (it = frozenTrain.begin();
         it != frozenTrain.end();
         ) {
        remainSeconds = it.value();
        remainSeconds--;
        it.value() = remainSeconds;
        if (remainSeconds <= 0) {
            qDebug() << "remove " << it.key() << " because timeout";
            it = frozenTrain.erase(it);
        } else {
            it++;
        }
    }
    if (frozenTrain.isEmpty()) {
        frozenTimer.stop();
    }
}
