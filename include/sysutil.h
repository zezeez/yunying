#ifndef SYSUTIL_H
#define SYSUTIL_H
#include <QDateTime>

class SysUtil
{
public:
    SysUtil();
    ~SysUtil();
    int setSysTime(const QDateTime &curTime);
    int setSysTime(const QString &curTime, int delay);
};

#endif // SYSUTIL_H
