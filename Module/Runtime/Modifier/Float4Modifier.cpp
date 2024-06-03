//==============================================================================
// Minamoto : Float4Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include "Float4Modifier.h"

//==============================================================================
//  Float4Modifier
//==============================================================================
void Float4Modifier::Set(xxVector4 const& value)
{
    Data.assign((char*)&value, (char*)&value + sizeof(xxVector4));
}
//------------------------------------------------------------------------------
xxVector4 Float4Modifier::Get() const
{
    xxVector4 value = xxVector4::ZERO;
    memcpy(&value, Data.data(), std::min(Data.size(), sizeof(xxVector4)));
    return value;
}
//------------------------------------------------------------------------------
xxModifierPtr Float4Modifier::Create(xxVector4 const& value)
{
    xxModifierPtr modifier = xxModifier::Create();
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, FLOAT4);
    auto ptr = (std::shared_ptr<Float4Modifier> const&)modifier;
    ptr->Set(value);

    return modifier;
}
//==============================================================================
