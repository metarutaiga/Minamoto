//==============================================================================
// Minamoto : TranslateModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>
#include "TranslateModifier.h"
#include "Modifier.inl"

//==============================================================================
//  TranslateModifier
//==============================================================================
void TranslateModifier::Update(void* target, xxModifierData* data, float time)
{
    Key* A;
    Key* B;
    float F;
    if (UpdateKeyFactor(data, time, A, B, F) == false)
        return;

    auto node = (xxNode*)target;
    node->SetTranslate(Lerp(A->translate, B->translate, F));
}
//------------------------------------------------------------------------------
xxModifierPtr TranslateModifier::Create(size_t count, std::function<void(size_t index, float& time, xxVector3& translate)> fill)
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
            fill(i, key[i].time, key[i].translate);
        }
    }
    return modifier;
}
//==============================================================================
