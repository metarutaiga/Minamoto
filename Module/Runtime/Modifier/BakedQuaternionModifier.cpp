//==============================================================================
// Minamoto : BakedQuaternionModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>
#include "BakedQuaternionModifier.h"
#include "Modifier.inl"

//==============================================================================
//  BakedQuaternionModifier
//==============================================================================
void BakedQuaternionModifier::Update(void* target, xxModifierData* data, float time)
{
    xxVector4* A;
    xxVector4* B;
    float F;
    if (UpdateBakedFactor(data, time, (Baked*)Data.data(), A, B, F) == false)
        return;

    auto node = (xxNode*)target;
    node->SetRotate(xxMatrix3::Quaternion(Lerp(*A, *B, F)));
}
//------------------------------------------------------------------------------
xxModifierPtr BakedQuaternionModifier::Create(size_t count, float duration, std::function<void(size_t index, xxVector4& quaternion)> fill)
{
    if (count <= 1)
        return nullptr;

    xxModifierPtr modifier = xxModifier::Create(sizeof(Baked) + sizeof(xxVector4) * count);
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, BAKED_QUATERNION);
    if (fill)
    {
        auto* baked = (Baked*)modifier->Data.data();
        baked->duration = duration;
        baked->frequency = duration / (count - 1);
        baked->inverseFrequency = 1.0f / baked->frequency;
        for (size_t i = 0; i < count; ++i)
        {
            fill(i, (xxVector4&)baked->values[i]);
        }
    }
    return modifier;
}
//==============================================================================
