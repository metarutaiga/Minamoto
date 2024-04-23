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
#include "BakedQuaternionModifier.h"
#include "Quaternion16Modifier.h"
#include "BakedQuaternion16Modifier.h"
#include "Modifier.h"

#define LOADER(class) reinterpret_cast<void(xxModifier::*)(void*, xxModifierData*, float)>(&class::Update)

static const struct { std::string name; xxModifier::UpdateFunction function; size_t header; size_t size; } loaders[] =
{
    [Modifier::UNKNOWN]             = { "UNKNOWN",              [](xxModifier*, void* target, xxModifierData* data, float time){}, 0, 1 },
    [Modifier::CONSTANT_QUATERNION] = { "CONSTANT_QUATERNION",  LOADER(ConstantQuaternionModifier), 0,                                          sizeof(ConstantQuaternionModifier::Constant) },
    [Modifier::CONSTANT_TRANSLATE]  = { "CONSTANT_TRANSLATE",   LOADER(ConstantTranslateModifier),  0,                                          sizeof(ConstantTranslateModifier::Constant) },
    [Modifier::CONSTANT_SCALE]      = { "CONSTANT_SCALE",       LOADER(ConstantScaleModifier),      0,                                          sizeof(ConstantScaleModifier::Constant) },
    [Modifier::QUATERNION]          = { "QUATERNION",           LOADER(QuaternionModifier),         0,                                          sizeof(QuaternionModifier::Key) },
    [Modifier::TRANSLATE]           = { "TRANSLATE",            LOADER(TranslateModifier),          0,                                          sizeof(TranslateModifier::Key) },
    [Modifier::SCALE]               = { "SCALE",                LOADER(ScaleModifier),              0,                                          sizeof(ScaleModifier::Key) },
    [Modifier::BAKED_QUATERNION]    = { "BAKED_QUATERNION",     LOADER(BakedQuaternionModifier),    sizeof(BakedQuaternionModifier::Baked),     sizeof(xxVector4) },
    [Modifier::QUATERNION16]        = { "QUATERNION16",         LOADER(Quaternion16Modifier),       0,                                          sizeof(Quaternion16Modifier::Key) },
    [Modifier::BAKED_QUATERNION16]  = { "BAKED_QUATERNION16",   LOADER(BakedQuaternion16Modifier),  sizeof(BakedQuaternion16Modifier::Baked),   sizeof(Modifier::v4hi) },
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

    size_t size = modifier.Data.size();
    if (size < loaders[type].header)
        return 0;
    return (size - loaders[type].header) / loaders[type].size;
}
//------------------------------------------------------------------------------
size_t Modifier::CalculateSize(size_t type, size_t count)
{
    if (type >= xxCountOf(loaders))
        type = UNKNOWN;

    return loaders[type].header + loaders[type].size * count;
}
//==============================================================================
