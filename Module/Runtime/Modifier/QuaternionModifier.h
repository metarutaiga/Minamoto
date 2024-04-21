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
        xxVector4 quaternion;
    };
#if defined(__aarch64__)
    static_assert(sizeof(Key) == 20);
#endif

public:
    void                    Update(void* target, xxModifierData* data, float time);

    static xxModifierPtr    Create(size_t count = 0, std::function<void(Key& key, size_t index)> fill = nullptr);
};
