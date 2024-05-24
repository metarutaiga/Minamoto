#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <time.h>

#define CLOCK_REALTIME 0
static int clock_gettime(int __clock, struct timespec* __ts)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const ULONGLONG EPOCH = 116444736000000000ULL;

    FILETIME ft;
    ULARGE_INTEGER time;

    GetSystemTimePreciseAsFileTime(&ft);
    time.LowPart = ft.dwLowDateTime;
    time.HighPart = ft.dwHighDateTime;
    time.QuadPart -= EPOCH;

    __ts->tv_sec = time.QuadPart / 10000000;
    __ts->tv_nsec = time.QuadPart % 10000000 * 100;
    return 0;
}

struct timezone;
static int gettimeofday(struct timeval* __tv, struct timezone* __tz)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    __tv->tv_sec = ts.tv_sec;
    __tv->tv_usec = ts.tv_nsec / 1000;
    return 0;
}
