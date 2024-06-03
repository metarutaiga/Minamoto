//==============================================================================
// Minamoto : ArrayModifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include "ArrayModifier.h"

//==============================================================================
//  ArrayModifier
//==============================================================================
void ArrayModifier::Set(std::span<char> array)
{
    Data.assign(array.data(), array.data() + array.size());
}
//------------------------------------------------------------------------------
std::span<char> ArrayModifier::Get() const
{
    return std::span<char>((char*)Data.data(), Data.size());
}
//------------------------------------------------------------------------------
xxModifierPtr ArrayModifier::Create(std::span<char> array)
{
    xxModifierPtr modifier = xxModifier::Create();
    if (modifier == nullptr)
        return nullptr;

    Loader(*modifier, ARRAY);
    if (array.empty() == false)
    {
        auto ptr = (std::shared_ptr<ArrayModifier> const&)modifier;
        ptr->Set(array);
    }
    return modifier;
}
//==============================================================================
