//==============================================================================
// Minamoto : QuaternionModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxNode.h>
#include "QuaternionModifier.h"
#include "Modifier.inl"

//==============================================================================
//  QuaternionModifier
//==============================================================================
void QuaternionModifier::Update(void* target, xxModifierData* data, float time)
{
    Key* A;
    Key* B;
    float X;
    float Y;
    if (GetUpdateRatio(data, time, A, B, X, Y) == false)
        return;
    auto node = (xxNode*)target;
    node->SetRotate(xxMatrix3::Quaternion(A->quaternion * X + B->quaternion * Y));
}
//------------------------------------------------------------------------------
xxModifierPtr QuaternionModifier::Create()
{
    xxModifierPtr modifier = xxModifier::Create();
    if (modifier == nullptr)
        return nullptr;

    ModifierLoader(*modifier, QUATERNION);
    return modifier;
}
//==============================================================================
