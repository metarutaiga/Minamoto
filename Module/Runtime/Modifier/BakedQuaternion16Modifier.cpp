//==============================================================================
// Minamoto : BakedQuaternion16Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>
#include "BakedQuaternion16Modifier.h"
#include "Modifier.inl"

//==============================================================================
//  BakedQuaternion16Modifier
//==============================================================================
void BakedQuaternion16Modifier::Update(void* target, xxModifierData* data, float time)
{
    v4hi* A;
    v4hi* B;
    float F;
    if (UpdateBakedFactor(data, time, (Baked*)Data.data(), A, B, F) == false)
        return;

    auto node = (xxNode*)target;
    xxVector4 L = { __builtin_convertvector(*A, v4sf) };
    xxVector4 R = { __builtin_convertvector(*B, v4sf) };
    node->SetRotate(xxMatrix3::Quaternion(Lerp(L, R, F) / 32767.0f));
}
//------------------------------------------------------------------------------
xxModifierPtr BakedQuaternion16Modifier::Create(size_t count, float duration, std::function<void(size_t index, xxVector4& quaternion)> fill)
{
    if (count <= 1)
        return nullptr;

    xxModifierPtr modifier = xxModifier::Create(sizeof(Baked) + sizeof(v4hi) * count);
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, BAKED_QUATERNION16);
    if (fill)
    {
        auto* baked = (Baked*)modifier->Data.data();
        baked->duration = duration;
        baked->frequency = duration / (count - 1);
        baked->inverseFrequency = 1.0f / baked->frequency;
        for (size_t i = 0; i < count; ++i)
        {
            xxVector4 quaternion;
            fill(i, quaternion);
            quaternion *= 32767.0f;
            (v4hi&)baked->values[i] = __builtin_convertvector(quaternion.v, v4hi);
        }
    }
    return modifier;
}
//==============================================================================
