//==============================================================================
// Minamoto : QuaternionModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>
#include "QuaternionModifier.h"
#include "Modifier.inl"

//==============================================================================
//  QuaternionModifier
//==============================================================================
void QuaternionModifier::Update(void* target, xxModifierData* data, float time)
{
    Key* A;
    Key* B;
    float F;
    if (UpdateKeyFactor(data, time, A, B, F) == false)
        return;

    auto node = (xxNode*)target;
    node->SetRotate(xxMatrix3::Quaternion(Lerp((xxVector4&)A->quaternion, (xxVector4&)B->quaternion, F)));
}
//------------------------------------------------------------------------------
xxModifierPtr QuaternionModifier::Create(size_t count, std::function<void(size_t index, float& time, xxVector4& quaternion)> fill)
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
            fill(i, key[i].time, (xxVector4&)key[i].quaternion);
        }
    }
    return modifier;
}
//==============================================================================
