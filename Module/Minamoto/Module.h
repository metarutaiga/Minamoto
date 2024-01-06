//==============================================================================
// Minamoto : Module Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Interface.h"

class Module
{
public:
    static void Create(const char* path, uint64_t device);
    static void Shutdown();

    static int Count();
    static void Message(std::initializer_list<const char*> list);
    static bool Update();
    static void Render();
};
