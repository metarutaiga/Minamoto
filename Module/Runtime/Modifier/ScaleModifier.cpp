//==============================================================================
// Minamoto : ScaleModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxNode.h>
#include "ScaleModifier.h"
#include "Modifier.inl"

//==============================================================================
//  ScaleModifier
//==============================================================================
void ScaleModifier::Update(void* target, xxModifierData* data, float time)
{
    Key* A;
    Key* B;
    float F;
    if (UpdateKeyFactor(data, time, A, B, F) == false)
        return;

    auto node = (xxNode*)target;
    node->SetScale(Lerp(A->scale, B->scale, F));
    node->UpdateRotateTranslateScale();
}
//------------------------------------------------------------------------------
xxModifierPtr ScaleModifier::Create(size_t count, std::function<void(size_t index, float& time, float& scale)> fill)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Key) * count);
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, SCALE);
    if (fill)
    {
        auto* key = (Key*)modifier->Data.data();
        for (size_t i = 0; i < count; ++i)
        {
            fill(i, key[i].time, key[i].scale);
        }
    }
    return modifier;
}
//==============================================================================
