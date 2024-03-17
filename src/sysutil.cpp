#include "sysutil.h"
#include <QDateTime>
#if defined(Q_OS_LINUX)
#include <sys/time.h>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

#define _ QStringLiteral

SysUtil::SysUtil()
{

}

SysUtil::~SysUtil()
{

}

int SysUtil::setSysTime(const QDateTime &newTime)
{
    int ret = 0;
#if defined(Q_OS_LINUX)
    std::tm time_tm = { };
    time_tm.tm_year = newTime.date().year() - 1900;  // 年份从1900开始
    time_tm.tm_mon = newTime.date().month() - 1;     // 月份从0开始（0-11）
    time_tm.tm_mday = newTime.date().day();          // 日期从1开始
    time_tm.tm_hour = newTime.time().hour();         // 0-23小时
    time_tm.tm_min = newTime.time().minute();        // 分钟0-59
    time_tm.tm_sec = newTime.time().second();        // 秒数0-59
    time_tm.tm_isdst = 0;                            // 不是夏令时

    // 将tm结构转换为time_t
    time_t now = mktime(&time_tm);

    // 设置系统时间
    struct timespec ts;
    ts.tv_sec = now;
    ts.tv_nsec = newTime.time().msec() / 1000000;

    ret = clock_settime(CLOCK_REALTIME, &ts);
    qDebug() << strerror(errno);
#elif defined(Q_OS_WIN)
    SYSTEMTIME st;
    GetSystemTime(&st);
    st.wYear = newTime.date().year();
    st.wMonth = newTime.date().month();
    st.wDay = newTime.date().day();

    st.wHour = newTime.time().hour();
    st.wMinute = newTime.time().minute();
    st.wSecond = newTime.time().second();
    st.wMilliseconds = newTime.time().msec();
    ret = SetLocalTime(&st);
#endif
    return ret;
}

int SysUtil::setSysTime(const QString &newTime, int delay)
{
    // Thu, 07 Mar 2024 02:03:28 GMT
    QDateTime toSetTime;
    int ret = -2;
    int last = newTime.lastIndexOf(' ');
    if (last != -1) {
        toSetTime.fromString(newTime.first(last), _("ddd, dd MMM yyyy hh:mm:ss"));
        if (toSetTime.isValid()) {
            toSetTime = toSetTime.addSecs(28800).addMSecs(delay);  // add 8 hour and delay
            QDateTime curTime = QDateTime::currentDateTime();
            curTime = curTime.addMSecs(-curTime.time().msec());
            if (curTime != toSetTime) {
                ret = setSysTime(toSetTime);
            }
        }
    }

    return ret;
}
