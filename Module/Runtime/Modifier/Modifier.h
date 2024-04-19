//==============================================================================
// Minamoto : Modifier Inline
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Runtime.h"
#include <utility/xxModifier.h>

class RuntimeAPI Modifier : public xxModifier
{
public:
    enum Type
    {
        UNKNOWN     = 0,
        QUATERNION  = 1,
        TRANSLATE   = 2,
        SCALE       = 3,
    };

public:
    template<class T>
    bool GetUpdateRatio(xxModifierData* data, float time, T*& A, T*& B, float& X, float& Y);

    static void Initialize();
    static void Shutdown();
    static void ModifierLoader(xxModifier& modifier, size_t type);
};
