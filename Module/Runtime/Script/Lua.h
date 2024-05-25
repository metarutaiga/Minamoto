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
    static char const* Version();

    static struct lua_State* L;
};
