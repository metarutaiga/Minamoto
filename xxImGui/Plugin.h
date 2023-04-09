//==============================================================================
// xxImGui : Plugin Header
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#pragma once

#include "plugin/interface.h"

class Plugin
{
public:
    static void Create(const char* path, uint64_t device);
    static void Shutdown();

    static int Count();
    static void Message(std::initializer_list<const char*> list);
    static bool Update();
    static void Render();
};
