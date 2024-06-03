//==============================================================================
// Minamoto : ArrayModifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI ArrayModifier : public Modifier
{
public:
    void                    Set(std::span<char> span);
    std::span<char>         Get() const;

    static xxModifierPtr    Create(std::span<char> span = {});
};
