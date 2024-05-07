//==============================================================================
// Minamoto : NodeTools Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Runtime.h"

struct RuntimeAPI NodeTools
{
    static xxNodePtr const& GetRoot(xxNodePtr const& node);
    static xxNodePtr const& GetObject(xxNodePtr const& node, std::string const& name);
    static void UpdateNodeFlags(xxNodePtr const& node);
};
