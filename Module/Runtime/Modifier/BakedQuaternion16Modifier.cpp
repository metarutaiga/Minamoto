//==============================================================================
// Minamoto : BakedQuaternion16Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxNode.h>
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
    xxVector4 L = xxVector4{ (float)(*A)[0], (float)(*A)[1], (float)(*A)[2], (float)(*A)[3] } /= 32767.0f;
    xxVector4 R = xxVector4{ (float)(*B)[0], (float)(*B)[1], (float)(*B)[2], (float)(*B)[3] } /= 32767.0f;
    node->SetRotate(xxMatrix3::Quaternion(Lerp(L, R, F)));
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
            baked->values[i][0] = quaternion.x;
            baked->values[i][1] = quaternion.y;
            baked->values[i][2] = quaternion.z;
            baked->values[i][3] = quaternion.w;
        }
    }
    return modifier;
}
//==============================================================================
