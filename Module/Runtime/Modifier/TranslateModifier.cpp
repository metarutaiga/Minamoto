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
    if (GetUpdateRatio(data, time, A, B, X, Y) == false)
        return;
    auto node = (xxNode*)target;
    node->SetTranslate(A->translate * X + B->translate * Y);
}
//------------------------------------------------------------------------------
xxModifierPtr TranslateModifier::Create()
{
    xxModifierPtr modifier = xxModifier::Create();
    if (modifier == nullptr)
        return nullptr;

    ModifierLoader(*modifier, TRANSLATE);
    return modifier;
}
//==============================================================================
