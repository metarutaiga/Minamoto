//==============================================================================
// Minamoto : QuaternionModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxNode.h>
#include "QuaternionModifier.h"
#include "Modifier.inl"

//==============================================================================
//  QuaternionModifier
//==============================================================================
void QuaternionModifier::Update(void* target, xxModifierData* data, float time)
{
    Key* A;
    Key* B;
    float X;
    float Y;
    if (UpdateRatio(data, time, A, B, X, Y) == false)
        return;
    auto node = (xxNode*)target;
    node->SetRotate(xxMatrix3::Quaternion(A->quaternion * X + B->quaternion * Y));
}
//------------------------------------------------------------------------------
xxModifierPtr QuaternionModifier::Create(size_t count, std::function<void(Key& key, size_t index)> fill)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Key) * count);
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, QUATERNION);
    if (fill)
    {
        auto* key = (Key*)modifier->Data.data();
        for (size_t i = 0; i < count; ++i)
        {
            fill(key[i], i);
        }
    }
    return modifier;
}
//==============================================================================
