//==============================================================================
// Minamoto : Float4Modifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI Float4Modifier : public Modifier
{
public:
    void                    Set(xxVector4 const& value);
    xxVector4               Get() const;

    static xxModifierPtr    Create(xxVector4 const& value = xxVector4::ZERO);
};
