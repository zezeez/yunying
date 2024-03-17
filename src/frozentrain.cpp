#include "frozentrain.h"
#include <QDebug>
FrozenTrain::FrozenTrain()
{
    frozenTimer.setInterval(1000);
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
        remainSeconds -= 1000;
        it.value() = remainSeconds;
        if (remainSeconds <= 0) {
            it = frozenTrain.erase(it);
        } else {
            it++;
        }
    }
    if (frozenTrain.isEmpty()) {
        frozenTimer.stop();
    }
}
