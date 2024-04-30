//==============================================================================
// Minamoto : BakedQuaternionModifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI BakedQuaternionModifier : public Modifier
{
public:
    struct Baked
    {
        float duration;
        float frequency;
        float inverseFrequency;
#if defined(_M_ARM) || defined(_M_ARM64) || defined(_M_IX86) || defined(_M_AMD64) 
        float values[][4];
#else
        xxVector4 values[];
#endif
    };
    static_assert(sizeof(Baked) == 12);

public:
    void                    Update(void* target, xxModifierData* data, float time);

    static xxModifierPtr    Create(size_t count = 0, float duration = 0.0f, std::function<void(size_t index, xxVector4& quaternion)> fill = nullptr);
};
