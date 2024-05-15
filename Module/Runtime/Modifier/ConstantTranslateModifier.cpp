//==============================================================================
// Minamoto : ConstantTranslateModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>
#include "ConstantTranslateModifier.h"

//==============================================================================
//  ConstantTranslateModifier
//==============================================================================
void ConstantTranslateModifier::Update(void* target, xxModifierData* data, float time)
{
    if (data->time == time)
        return;
    data->time = time;

    auto node = (xxNode*)target;
    auto* constant = (Constant*)Data.data();
    node->SetTranslate(constant->translate);
}
//------------------------------------------------------------------------------
xxModifierPtr ConstantTranslateModifier::Create(xxVector3 const& translate)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Constant));
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, CONSTANT_TRANSLATE);
    auto* constant = (Constant*)modifier->Data.data();
    constant->translate = translate;
    return modifier;
}
//==============================================================================
