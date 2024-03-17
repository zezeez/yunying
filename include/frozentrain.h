#ifndef FROZENTRAIN_H
#define FROZENTRAIN_H
#include <QMap>
#include <QString>
#include <QTimer>

class FrozenTrain
{
public:
    FrozenTrain();
    ~FrozenTrain();
    void addFrozenTrain(const QString &trainDesc, int seconds);
    void removeFrozenTrain(const QString &trainDesc);
    bool queryTrainIsFrozen(const QString &trainDesc);
    void detectFrozenTimeout();
private:
    QMap<QString, int> frozenTrain;
    QTimer frozenTimer;
};

#endif // FROZENTRAIN_H
