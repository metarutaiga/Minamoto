//==============================================================================
// Minamoto : QuickJS Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <queue>
#include <string>
#include "Runtime.h"

struct RuntimeAPI QuickJS
{
    static void Initialize();
    static void Shutdown();
    static void Input(char c);
    static void Eval(uint8_t const* buf, size_t len);
    static void Update();

    static struct JSRuntime* rt;
    static struct JSContext* ctx;
    static std::deque<char> Inputs;
    static std::deque<std::string> Outputs;
};
