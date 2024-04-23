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
    xxVector4 L = xxVector4{ (float)A->quaternion[0], (float)A->quaternion[1], (float)A->quaternion[2], (float)A->quaternion[3] } /= 32767.0f;
    xxVector4 R = xxVector4{ (float)B->quaternion[0], (float)B->quaternion[1], (float)B->quaternion[2], (float)B->quaternion[3] } /= 32767.0f;
    node->SetRotate(xxMatrix3::Quaternion(Lerp(L, R, F)));
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
            key[i].quaternion[0] = quaternion.x;
            key[i].quaternion[1] = quaternion.y;
            key[i].quaternion[2] = quaternion.z;
            key[i].quaternion[3] = quaternion.w;
        }
    }
    return modifier;
}
//==============================================================================
