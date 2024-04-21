//==============================================================================
// Minamoto : ConstantTranslateModifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI ConstantTranslateModifier : public Modifier
{
public:
    struct Constant
    {
        xxVector3 translate;
    };

public:
    void                    Update(void* target, xxModifierData* data, float time);

    static xxModifierPtr    Create(xxVector3 const& translate = xxVector3::ZERO);
};
