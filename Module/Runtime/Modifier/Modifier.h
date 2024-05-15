//==============================================================================
// Minamoto : Modifier Inline
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Runtime.h"
#include <xxGraphicPlus/xxModifier.h>

class RuntimeAPI Modifier : public xxModifier
{
public:
    enum Type
    {
        UNKNOWN             =  0,
        CONSTANT_QUATERNION = 10,
        CONSTANT_TRANSLATE  = 11,
        CONSTANT_SCALE      = 12,
        QUATERNION          = 20,
        TRANSLATE           = 21,
        SCALE               = 22,
        BAKED_QUATERNION    = 30,
        BAKED_TRANSLATE     = 31,
        BAKED_SCALE         = 32,
        QUATERNION16        = 50,
        BAKED_QUATERNION16  = 51,
    };

public:
    template<class T> bool          UpdateKeyFactor(xxModifierData* data, float time, T*& A, T*& B, float& F);
    template<class T, class D> bool UpdateBakedFactor(xxModifierData* data, float time, D* baked, T*& A, T*& B, float& F);
    template<class T> T             Lerp(T const& A, T const &B, float F);

    static void                     Initialize();
    static void                     Shutdown();
    static void                     Loader(xxModifier& modifier, size_t type);
    static std::string const&       Name(xxModifier& modifier);
    static size_t                   Count(xxModifier& modifier);
    static size_t                   CalculateSize(size_t type, size_t count);
};
