//==============================================================================
// Minamoto : Quaternion16Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxNode.h>
#include "Quaternion16Modifier.h"
#include "Modifier.inl"

//==============================================================================
//  Quaternion16Modifier
//==============================================================================
void Quaternion16Modifier::Update(void* target, xxModifierData* data, float time)
{
    Key* A;
    Key* B;
    float F;
    if (UpdateKeyFactor(data, time, A, B, F) == false)
        return;

    auto node = (xxNode*)target;
    xxVector4 L = { __builtin_convertvector((v4hi&)A->quaternion, v4sf) };
    xxVector4 R = { __builtin_convertvector((v4hi&)B->quaternion, v4sf) };
    node->SetRotate(xxMatrix3::Quaternion(Lerp(L, R, F) / 32767.0f));
}
//------------------------------------------------------------------------------
xxModifierPtr Quaternion16Modifier::Create(size_t count, std::function<void(size_t index, float& time, xxVector4& quaternion)> fill)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Key) * count);
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, QUATERNION16);
    if (fill)
    {
        auto* key = (Key*)modifier->Data.data();
        for (size_t i = 0; i < count; ++i)
        {
            float time;
            xxVector4 quaternion;
            fill(i, time, quaternion);
            quaternion *= 32767.0f;
            key[i].time = time;
            (v4hi&)key[i].quaternion = __builtin_convertvector(quaternion.v, v4hi);
        }
    }
    return modifier;
}
//==============================================================================
