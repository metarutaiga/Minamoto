//==============================================================================
// Minamoto : Buffer Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Runtime.h"

struct RuntimeAPI Buffer
{
    static void Initialize();
    static void Update();
    static void Shutdown();
};
