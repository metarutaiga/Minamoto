//==============================================================================
// Minamoto : FloatModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include "FloatModifier.h"

//==============================================================================
//  FloatModifier
//==============================================================================
void FloatModifier::Set(float value)
{
    Data.assign((char*)&value, (char*)&value + sizeof(float));
}
//------------------------------------------------------------------------------
float FloatModifier::Get() const
{
    float value = 0.0f;
    memcpy(&value, Data.data(), std::min(Data.size(), sizeof(float)));
    return value;
}
//------------------------------------------------------------------------------
xxModifierPtr FloatModifier::Create(float value)
{
    xxModifierPtr modifier = xxModifier::Create();
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, FLOAT);
    auto ptr = (std::shared_ptr<FloatModifier> const&)modifier;
    ptr->Set(value);

    return modifier;
}
//==============================================================================
