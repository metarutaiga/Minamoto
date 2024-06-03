//==============================================================================
// Minamoto : StringModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include "StringModifier.h"

//==============================================================================
//  StringModifier
//==============================================================================
void StringModifier::Set(std::string_view const& string)
{
    Data.assign(string.data(), string.data() + string.size());
}
//------------------------------------------------------------------------------
std::string_view StringModifier::Get() const
{
    return std::string_view(Data.data(), Data.size());
}
//------------------------------------------------------------------------------
xxModifierPtr StringModifier::Create(std::string_view const& string)
{
    xxModifierPtr modifier = xxModifier::Create();
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, STRING);
    if (string.empty() == false)
    {
        auto ptr = (std::shared_ptr<StringModifier> const&)modifier;
        ptr->Set(string);
    }
    return modifier;
}
//==============================================================================
