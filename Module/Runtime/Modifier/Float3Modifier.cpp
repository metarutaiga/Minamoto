//==============================================================================
// Minamoto : Float3Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include "Float3Modifier.h"

//==============================================================================
//  Float3Modifier
//==============================================================================
void Float3Modifier::Set(xxVector3 const& value)
{
    Data.assign((char*)&value, (char*)&value + sizeof(xxVector3));
}
//------------------------------------------------------------------------------
xxVector3 Float3Modifier::Get() const
{
    xxVector3 value = xxVector3::ZERO;
    memcpy(&value, Data.data(), std::min(Data.size(), sizeof(xxVector3)));
    return value;
}
//------------------------------------------------------------------------------
xxModifierPtr Float3Modifier::Create(xxVector3 const& value)
{
    xxModifierPtr modifier = xxModifier::Create();
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, FLOAT3);
    auto ptr = (std::shared_ptr<Float3Modifier> const&)modifier;
    ptr->Set(value);

    return modifier;
}
//==============================================================================
