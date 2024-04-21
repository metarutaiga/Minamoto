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
    float X;
    float Y;
    if (GetUpdateRatio(data, time, A, B, X, Y) == false)
        return;
    auto node = (xxNode*)target;
    node->SetScale(A->scale * X + B->scale * Y);
}
//------------------------------------------------------------------------------
xxModifierPtr ScaleModifier::Create(size_t count, std::function<void(Key& key, size_t index)> fill)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Key) * count);
    if (modifier == nullptr)
        return nullptr;

    ModifierLoader(*modifier, SCALE);
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
