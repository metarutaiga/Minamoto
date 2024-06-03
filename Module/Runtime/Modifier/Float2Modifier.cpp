//==============================================================================
// Minamoto : Float2Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include "Float2Modifier.h"

//==============================================================================
//  Float2Modifier
//==============================================================================
void Float2Modifier::Set(xxVector2 const& value)
{
    Data.assign((char*)&value, (char*)&value + sizeof(xxVector2));
}
//------------------------------------------------------------------------------
xxVector2 Float2Modifier::Get() const
{
    xxVector2 value = xxVector2::ZERO;
    memcpy(&value, Data.data(), std::min(Data.size(), sizeof(xxVector2)));
    return value;
}
//------------------------------------------------------------------------------
xxModifierPtr Float2Modifier::Create(xxVector2 const& value)
{
    xxModifierPtr modifier = xxModifier::Create();
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, FLOAT2);
    auto ptr = (std::shared_ptr<Float2Modifier> const&)modifier;
    ptr->Set(value);

    return modifier;
}
//==============================================================================
