//==============================================================================
// Minamoto : ImportWavefront Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <map>
#include "Import.h"

class ImportWavefront : public Import
{
    struct Material
    {
        xxMaterialPtr output;
        xxTexturePtr map_Ka;
        xxTexturePtr map_Kd;
        xxTexturePtr map_Ks;
        xxTexturePtr map_Ns;
        xxTexturePtr map_d;
        xxTexturePtr decal;
        xxTexturePtr disp;
        xxTexturePtr bump;
    };

public:
    static std::map<std::string, Material> CreateMaterial(char const* mtl);
    static xxNodePtr Create(char const* obj);
};
