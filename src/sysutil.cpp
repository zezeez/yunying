#include "sysutil.h"
#include "mainwindow.h"
#include <QDateTime>
#if defined(Q_OS_LINUX)
#include <sys/time.h>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

#define _ QStringLiteral

extern MainWindow *w;

SysUtil::SysUtil()
{

}

SysUtil::~SysUtil()
{

}

int SysUtil::setSysTime(const QDateTime &newTime)
{
    int ret = 0;

#if defined(Q_OS_WIN)
    SYSTEMTIME st;
    /*HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    //获得SE_SYSTEMTIME_NAME权限//
    // Get a token for this process.
    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return( FALSE );

    //(获得SE_SYSTEMTIME_NAME权限，才可设置系统时间成功)
    LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME,
                         &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp),
                          (PTOKEN_PRIVILEGES)NULL, 0);

    qDebug() << GetLastError();
    if (GetLastError() != ERROR_SUCCESS) {
        CloseHandle(hToken);
        return 0;
    }*/

    /*STARTUPINFO si;
    PROCESS_INFORMATION pi;
    LPWSTR s = {};
    wsprintfW(s, L"123");
    // Start the child process.
    if( !CreateProcess( 0,   // No module name (use command line)
                       s,        // Command line
                       0,           // Process handle not inheritable
                       0,           // Thread handle not inheritable
                       FALSE,          // Set handle inheritance to FALSE
                       0,              // No creation flags
                       0,           // Use parent's environment block
                       0,           // Use parent's starting directory
                       &si,            // Pointer to STARTUPINFO structure
                       &pi )           // Pointer to PROCESS_INFORMATION structure
        )
    {

    }*/
    GetSystemTime(&st);
    st.wYear = newTime.date().year();
    st.wMonth = newTime.date().month();
    st.wDay = newTime.date().day();

    st.wHour = newTime.time().hour();
    st.wMinute = newTime.time().minute();
    st.wSecond = newTime.time().second();
    st.wMilliseconds = newTime.time().msec();
    ret = SetLocalTime(&st);
    if (ret == 0) {
        qDebug() << "set systime error: " << GetLastError();
        w->formatOutput("更新系统时间失败，可能是没有足够的权限");
    } else {
        w->formatOutput("更新系统时间成功");
    }
    qDebug() << "set local time ret: " << ret;
    //CloseHandle(hToken);
#else
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
    ts.tv_nsec = newTime.time().msec() / 1e6;

    ret = clock_settime(CLOCK_REALTIME, &ts);
    if (ret < 0) {
        if (errno == EPERM) {
            w->formatOutput(_("更新系统时间失败，原因：没有足够的权限"));
        } else if (errno == EINVAL) {
            w->formatOutput(_("更新系统时间失败，原因：参数错误"));
        } else {
            w->formatOutput(_("更新系统时间失败，原因：") + strerror(errno));
        }
    } else {
        w->formatOutput("更新系统时间成功");
    }
#endif
    return ret;
}
