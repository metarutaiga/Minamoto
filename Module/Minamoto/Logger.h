//==============================================================================
// Minamoto : Logger Header
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <queue>

class Logger
{
public:
    static void Init();
    static void Shutdown();
    static void Printf(char const* tag, char const* format, va_list list);
    static void Update(std::deque<char*>& log);
};
