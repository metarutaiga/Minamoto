//==============================================================================
// Minamoto : ConstantScaleModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxNode.h>
#include "ConstantScaleModifier.h"

//==============================================================================
//  ConstantScaleModifier
//==============================================================================
void ConstantScaleModifier::Update(void* target, xxModifierData* data, float time)
{
    auto node = (xxNode*)target;
    Constant* constant = (Constant*)Data.data();
    node->SetScale(constant->scale);
}
//------------------------------------------------------------------------------
xxModifierPtr ConstantScaleModifier::Create(float scale)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Constant));
    if (modifier == nullptr)
        return nullptr;

    ModifierLoader(*modifier, CONSTANT_SCALE);
    auto* constant = (Constant*)modifier->Data.data();
    constant->scale = scale;
    return modifier;
}
//==============================================================================
