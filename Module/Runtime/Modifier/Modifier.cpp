//==============================================================================
// Minamoto : Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "ConstantQuaternionModifier.h"
#include "ConstantScaleModifier.h"
#include "ConstantTranslateModifier.h"
#include "QuaternionModifier.h"
#include "ScaleModifier.h"
#include "TranslateModifier.h"
#include "Modifier.h"

#define LOADER(class) reinterpret_cast<void(xxModifier::*)(void*, xxModifierData*, float)>(&class::Update)

static const struct { std::string name; xxModifier::UpdateFunction function; size_t size; } loaders[] =
{
    [Modifier::UNKNOWN]             = { "UNKNOWN",              [](xxModifier*, void* target, xxModifierData* data, float time){}, 1 },
    [Modifier::CONSTANT_QUATERNION] = { "CONSTANT_QUATERNION",  LOADER(ConstantQuaternionModifier), sizeof(ConstantQuaternionModifier::Constant) },
    [Modifier::CONSTANT_TRANSLATE]  = { "CONSTANT_TRANSLATE",   LOADER(ConstantTranslateModifier), sizeof(ConstantTranslateModifier::Constant) },
    [Modifier::CONSTANT_SCALE]      = { "CONSTANT_SCALE",       LOADER(ConstantScaleModifier), sizeof(ConstantScaleModifier::Constant) },
    [Modifier::QUATERNION]          = { "QUATERNION",           LOADER(QuaternionModifier), sizeof(QuaternionModifier::Key) },
    [Modifier::TRANSLATE]           = { "TRANSLATE",            LOADER(TranslateModifier), sizeof(TranslateModifier::Key) },
    [Modifier::SCALE]               = { "SCALE",                LOADER(ScaleModifier), sizeof(ScaleModifier::Key) },
};
//==============================================================================
void Modifier::Initialize()
{
    xxModifier::Loader = Modifier::Loader;
}
//------------------------------------------------------------------------------
void Modifier::Shutdown()
{
    xxModifier::Loader = [](xxModifier&, size_t){};
}
//------------------------------------------------------------------------------
void Modifier::Loader(xxModifier& modifier, size_t type)
{
    if (type >= xxCountOf(loaders) || loaders[type].function == nullptr)
        type = UNKNOWN;
    const_cast<xxModifier::UpdateFunction&>(modifier.Update) = loaders[type].function;
    if (type != UNKNOWN)
        const_cast<size_t&>(modifier.DataType) = type;
}
//------------------------------------------------------------------------------
std::string const& Modifier::Name(xxModifier& modifier)
{
    size_t type = modifier.DataType;
    if (type >= xxCountOf(loaders))
        type = UNKNOWN;
    return loaders[type].name;
}
//------------------------------------------------------------------------------
size_t Modifier::Count(xxModifier& modifier)
{
    size_t type = modifier.DataType;
    if (type >= xxCountOf(loaders))
        type = UNKNOWN;
    return modifier.Data.size() / loaders[type].size;
}
//==============================================================================
