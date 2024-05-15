//==============================================================================
// Minamoto : ConstantQuaternionModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>
#include "ConstantQuaternionModifier.h"

//==============================================================================
//  ConstantQuaternionModifier
//==============================================================================
void ConstantQuaternionModifier::Update(void* target, xxModifierData* data, float time)
{
    if (data->time == time)
        return;
    data->time = time;

    auto node = (xxNode*)target;
    auto* constant = (Constant*)Data.data();
    node->SetRotate(xxMatrix3::Quaternion(constant->quaternion));
}
//------------------------------------------------------------------------------
xxModifierPtr ConstantQuaternionModifier::Create(xxVector4 const& quaternion)
{
    xxModifierPtr modifier = xxModifier::Create(sizeof(Constant));
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, CONSTANT_QUATERNION);
    auto* constant = (Constant*)modifier->Data.data();
    constant->quaternion = quaternion;
    return modifier;
}
//==============================================================================
