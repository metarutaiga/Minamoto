//==============================================================================
// Minamoto : Float3Modifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI Float3Modifier : public Modifier
{
public:
    void                    Set(xxVector3 const& value);
    xxVector3               Get() const;

    static xxModifierPtr    Create(xxVector3 const& value = xxVector3::ZERO);
};
