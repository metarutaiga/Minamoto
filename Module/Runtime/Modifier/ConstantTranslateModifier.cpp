//==============================================================================
// Minamoto : ConstantTranslateModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxNode.h>
#include "ConstantTranslateModifier.h"

//==============================================================================
//  ConstantTranslateModifier
//==============================================================================
void ConstantTranslateModifier::Update(void* target, xxModifierData* data, float time)
{
    auto node = (xxNode*)target;
    Constant* constant = (Constant*)Data.data();
    node->SetTranslate(constant->translate);
}
//------------------------------------------------------------------------------
xxModifierPtr ConstantTranslateModifier::Create(xxVector3 const& translate)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Constant));
    if (modifier == nullptr)
        return nullptr;

    ModifierLoader(*modifier, CONSTANT_TRANSLATE);
    auto* constant = (Constant*)modifier->Data.data();
    constant->translate = translate;
    return modifier;
}
//==============================================================================
