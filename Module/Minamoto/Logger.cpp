//==============================================================================
// Minamoto : Logger Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <xxGraphic/xxSystem.h>
#include <string.h>
#include <mutex>
#include "Logger.h"

#if defined(xxWINDOWS)
#include <windows.h>
#endif

static char logAccumLockMemory[sizeof(std::mutex)];
static std::mutex& logAccumLock = *(std::mutex*)logAccumLockMemory;
static std::deque<char*> logAccum;
//------------------------------------------------------------------------------
void Logger::Create()
{
    new (&logAccumLock) std::mutex;
    xxLog = Logger::Printf;
}
//------------------------------------------------------------------------------
void Logger::Shutdown()
{
    logAccumLock.lock();
    for (char* line : logAccum)
        xxFree(line);
    logAccum.clear();
    logAccumLock.unlock();
    logAccumLock.~mutex();
}
//------------------------------------------------------------------------------
void Logger::Printf(char const* tag, char const* format, ...)
{
    char fmt[256];
    snprintf(fmt, 256, "[%s] %s\n", tag, format);

    va_list va;
    va_start(va, format);
    int size = vsnprintf(nullptr, 0, fmt, va);
    va_end(va);

    char* temp = xxAlloc(char, size + 1);
    if (temp == nullptr)
        return;

    va_start(va, format);
    vsnprintf(temp, size + 1, fmt, va);
    va_end(va);

#if defined(_WIN32)
    OutputDebugStringA(temp);
#else
    va_start(va, format);
    vprintf(fmt, va);
    va_end(va);
#endif

    char* lasts = temp;
    char* text = strsep(&lasts, "\n");
    while (text)
    {
        size_t length = strlen(text);
        if (length != 0)
        {
            char* line = xxAlloc(char, 9 + length + 1);
            if (line)
            {
                time_t t = time(nullptr);
                struct tm* tm = localtime(&t);
                snprintf(line, 9 + length + 1, "%02d:%02d:%02d %s", tm->tm_hour, tm->tm_min, tm->tm_sec, text);

                logAccumLock.lock();
                logAccum.push_back(line);
                logAccumLock.unlock();
            }
        }
        text = strsep(&lasts, "\n");
    }

    xxFree(temp);
}
//------------------------------------------------------------------------------
void Logger::Update(std::deque<char*>& log)
{
    if (logAccum.empty())
        return;
    if (logAccumLock.try_lock() == false)
        return;
    log.insert(log.end(), logAccum.begin(), logAccum.end());
    logAccum.clear();
    logAccumLock.unlock();
}
//------------------------------------------------------------------------------
