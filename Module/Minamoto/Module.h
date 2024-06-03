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
    static void Create(char const* path, uint64_t device);
    static void Shutdown();

    static int Count();
    static void Message(std::initializer_list<char const*> list);
    static bool Update();
    static void Render();
};
