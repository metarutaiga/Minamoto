//==============================================================================
// Minamoto : Lua Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <queue>
#include <string>
#include "Runtime.h"

struct RuntimeAPI Lua
{
    static void Initialize();
    static void Shutdown();
    static void RuntimeLibrary();
    static void Eval(char const* buf, size_t len);
    static void Update();

    static struct lua_State* L;
    static char const Version[];
};
