//==============================================================================
// Minamoto : BakedQuaternion16Modifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI BakedQuaternion16Modifier : public Modifier
{
public:
    struct Baked
    {
        float duration;
        float frequency;
        float inverseFrequency;
        v4hi values[];
    };
#if defined(__aarch64__)
    static_assert(sizeof(Baked) == 12);
#endif

public:
    void                    Update(void* target, xxModifierData* data, float time);

    static xxModifierPtr    Create(size_t count = 0, float duration = 0.0f, std::function<void(size_t index, xxVector4& quaternion)> fill = nullptr);
};
