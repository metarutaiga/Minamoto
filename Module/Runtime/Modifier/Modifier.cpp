//==============================================================================
// Minamoto : Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include "FloatModifier.h"
#include "Float2Modifier.h"
#include "Float3Modifier.h"
#include "Float4Modifier.h"
#include "StringModifier.h"
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
    { "UNKNOWN",              [](xxModifier*, void*, xxModifierData*, float) {}, 0,                         1 },
    { "FLOAT",                [](xxModifier*, void*, xxModifierData*, float) {}, 0,                         sizeof(float) },
    { "FLOAT2",               [](xxModifier*, void*, xxModifierData*, float) {}, 0,                         sizeof(xxVector2) },
    { "FLOAT3",               [](xxModifier*, void*, xxModifierData*, float) {}, 0,                         sizeof(xxVector3) },
    { "FLOAT4",               [](xxModifier*, void*, xxModifierData*, float) {}, 0,                         sizeof(xxVector4) },
    { "ARRAY",                [](xxModifier*, void*, xxModifierData*, float) {}, 0,                         sizeof(char) },
    { "STRING",               [](xxModifier*, void*, xxModifierData*, float) {}, 0,                         sizeof(char) },
                                {}, {}, {},
    { "CONSTANT_QUATERNION",  LOADER(ConstantQuaternionModifier), 0,                                        sizeof(ConstantQuaternionModifier::Constant) },
    { "CONSTANT_TRANSLATE",   LOADER(ConstantTranslateModifier),  0,                                        sizeof(ConstantTranslateModifier::Constant) },
    { "CONSTANT_SCALE",       LOADER(ConstantScaleModifier),      0,                                        sizeof(ConstantScaleModifier::Constant) },
                {}, {}, {}, {}, {}, {}, {},
    { "QUATERNION",           LOADER(QuaternionModifier),         0,                                        sizeof(QuaternionModifier::Key) },
    { "TRANSLATE",            LOADER(TranslateModifier),          0,                                        sizeof(TranslateModifier::Key) },
    { "SCALE",                LOADER(ScaleModifier),              0,                                        sizeof(ScaleModifier::Key) },
                {}, {}, {}, {}, {}, {}, {},
    { "BAKED_QUATERNION",     LOADER(BakedQuaternionModifier),    sizeof(BakedQuaternionModifier::Baked),   sizeof(xxVector4) },
        {}, {}, {}, {}, {}, {}, {}, {}, {},
    {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    { "QUATERNION16",         LOADER(Quaternion16Modifier),       0,                                        sizeof(Quaternion16Modifier::Key) },
    { "BAKED_QUATERNION16",   LOADER(BakedQuaternion16Modifier),  sizeof(BakedQuaternion16Modifier::Baked), sizeof(v4hi) },
};
static_assert(xxCountOf(loaders) == Modifier::BAKED_QUATERNION16 + 1);
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
