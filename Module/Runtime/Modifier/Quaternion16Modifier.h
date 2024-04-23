//==============================================================================
// Minamoto : Quaternion16Modifier Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Modifier.h"

class RuntimeAPI Quaternion16Modifier : public Modifier
{
public:
    struct Key
    {
        float time;
        v4hi quaternion;
    };
    static_assert(sizeof(Key) == 12);

public:
    void                    Update(void* target, xxModifierData* data, float time);

    static xxModifierPtr    Create(size_t count = 0, std::function<void(size_t index, float& time, xxVector4& quaternion)> fill = nullptr);
};
