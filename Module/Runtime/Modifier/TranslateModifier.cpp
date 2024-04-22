//==============================================================================
// Minamoto : TranslateModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxNode.h>
#include "TranslateModifier.h"
#include "Modifier.inl"

//==============================================================================
//  TranslateModifier
//==============================================================================
void TranslateModifier::Update(void* target, xxModifierData* data, float time)
{
    Key* A;
    Key* B;
    float X;
    float Y;
    if (UpdateRatio(data, time, A, B, X, Y) == false)
        return;
    auto node = (xxNode*)target;
    node->SetTranslate(A->translate * X + B->translate * Y);
}
//------------------------------------------------------------------------------
xxModifierPtr TranslateModifier::Create(size_t count, std::function<void(Key& key, size_t index)> fill)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Key) * count);
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, TRANSLATE);
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
