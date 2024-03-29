#ifndef SYSUTIL_H
#define SYSUTIL_H
#include <QDateTime>

class SysUtil
{
public:
    SysUtil();
    ~SysUtil();
    static int setSysTime(const QDateTime &curTime);
};

#endif // SYSUTIL_H
