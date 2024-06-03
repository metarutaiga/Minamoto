//==============================================================================
// Minamoto : Float2Modifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI Float2Modifier : public Modifier
{
public:
    void                    Set(xxVector2 const& value);
    xxVector2               Get() const;

    static xxModifierPtr    Create(xxVector2 const& value = xxVector2::ZERO);
};
