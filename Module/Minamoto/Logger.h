//==============================================================================
// Minamoto : Logger Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <queue>

class Logger
{
public:
    static void Create();
    static void Shutdown();
    static void Printf(char const* tag, char const* format, va_list list);
    static void Update(std::deque<char*>& log);
};
