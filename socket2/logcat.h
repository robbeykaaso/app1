#ifndef LOGCAT_H
#define LOGCAT_H

#define OpenDebug

#if defined (OpenDebug)
#include <QDebug>
#include <QDateTime>
#define datetime (QDateTime::currentDateTime().toString("yyyy-MM-dd:hh:mm:ss:zzz").toLatin1().data())
#define mDebug(fmt, ...) do { \
    qDebug("[ %s %s %s line = %d ]"#fmt, datetime, __FILE__, Q_FUNC_INFO, __LINE__, ##__VA_ARGS__); \
} while (0)

#else
#define mDebug(fmt, ...)
#endif
#endif // LOGCAT_H
