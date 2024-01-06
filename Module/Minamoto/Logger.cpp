//==============================================================================
// Minamoto : Logger Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <xxGraphic/xxSystem.h>
#include <string.h>
#include <mutex>
#include "Logger.h"

static std::deque<char*> logAccum;
static xxMutex logAccumLock;
//------------------------------------------------------------------------------
void Logger::Create()
{
    xxMutexInit(&logAccumLock);
    xxLogCallback(Logger::Printf);
}
//------------------------------------------------------------------------------
void Logger::Shutdown()
{
    xxMutexLock(&logAccumLock);
    for (char* line : logAccum)
        xxFree(line);
    logAccum.clear();
    xxMutexUnlock(&logAccumLock);
    xxMutexDestroy(&logAccumLock);
}
//------------------------------------------------------------------------------
void Logger::Printf(char const* tag, char const* format, va_list list)
{
    char fmt[256];
    snprintf(fmt, 256, "[%s] %s\n", tag, format);

    int size = vsnprintf(nullptr, 0, fmt, list);
    char* temp = xxAlloc(char, size + 1);
    if (temp == nullptr)
        return;

    vsnprintf(temp, size + 1, fmt, list);
#if defined(_WIN32)
    OutputDebugStringA(temp);
    OutputDebugStringA("\n");
#else
    vprintf(fmt, list);
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

                xxMutexLock(&logAccumLock);
                logAccum.push_back(line);
                xxMutexUnlock(&logAccumLock);
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
    if (xxMutexTryLock(&logAccumLock) == false)
        return;
    log.insert(log.end(), logAccum.begin(), logAccum.end());
    logAccum.clear();
    xxMutexUnlock(&logAccumLock);
}
//------------------------------------------------------------------------------
