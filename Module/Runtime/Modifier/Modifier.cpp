//==============================================================================
// Minamoto : Modifier Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "QuaternionModifier.h"
#include "ScaleModifier.h"
#include "TranslateModifier.h"
#include "Modifier.h"

//==============================================================================
void Modifier::Initialize()
{
    xxModifier::ModifierLoader = Modifier::ModifierLoader;
}
//------------------------------------------------------------------------------
void Modifier::Shutdown()
{
    xxModifier::ModifierLoader = [](xxModifier&, size_t){};
}
//------------------------------------------------------------------------------
void Modifier::ModifierLoader(xxModifier& modifier, size_t type)
{
    switch (type)
    {
    default:
    case UNKNOWN:
        const_cast<xxModifier::UpdateFunction&>(modifier.Update) = [](xxModifier*, void* target, xxModifierData* data, float time){};
//      const_cast<size_t&>(modifier.DataType) = UNKNOWN;
        break;
    case QUATERNION:
        const_cast<xxModifier::UpdateFunction&>(modifier.Update) = reinterpret_cast<void(xxModifier::*)(void*, xxModifierData*, float)>(&QuaternionModifier::Update);
        const_cast<size_t&>(modifier.DataType) = QUATERNION;
        break;
    case TRANSLATE:
        const_cast<xxModifier::UpdateFunction&>(modifier.Update) = reinterpret_cast<void(xxModifier::*)(void*, xxModifierData*, float)>(&TranslateModifier::Update);
        const_cast<size_t&>(modifier.DataType) = TRANSLATE;
        break;
    case SCALE:
        const_cast<xxModifier::UpdateFunction&>(modifier.Update) = reinterpret_cast<void(xxModifier::*)(void*, xxModifierData*, float)>(&ScaleModifier::Update);
        const_cast<size_t&>(modifier.DataType) = SCALE;
        break;
    }
}
//==============================================================================
