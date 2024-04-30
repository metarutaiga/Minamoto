//==============================================================================
// Minamoto : QuaternionModifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI QuaternionModifier : public Modifier
{
public:
    struct Key
    {
        float time;
#if defined(_M_ARM) || defined(_M_ARM64) || defined(_M_IX86) || defined(_M_AMD64) 
        float quaternion[4];
#else
        xxVector4 quaternion;
#endif
    };
    static_assert(sizeof(Key) == 20);

public:
    void                    Update(void* target, xxModifierData* data, float time);

    static xxModifierPtr    Create(size_t count = 0, std::function<void(size_t index, float& time, xxVector4& quaternion)> fill = nullptr);
};
