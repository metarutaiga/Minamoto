#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

struct timezone;
static int gettimeofday(struct timeval* __tv, struct timezone* __tz)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970
    static const ULONGLONG EPOCH = 116444736000000000ULL;

    SYSTEMTIME system_time;
    FILETIME file_time;
    ULONGLONG time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((ULONGLONG)file_time.dwLowDateTime);
    time += ((ULONGLONG)file_time.dwHighDateTime) << 32;

    __tv->tv_sec = (long)((time - EPOCH) / 10000000L);
    __tv->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}

#define clock_gettime(a,b) { (b)->tv_sec = 0; (b)->tv_nsec = 0; }
