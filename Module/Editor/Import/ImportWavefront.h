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
        xxImagePtr map_Ka;
        xxImagePtr map_Kd;
        xxImagePtr map_Ks;
        xxImagePtr map_Ns;
        xxImagePtr map_d;
        xxImagePtr decal;
        xxImagePtr disp;
        xxImagePtr bump;
    };

public:
    static std::map<std::string, Material> CreateMaterial(char const* mtl);
    static xxNodePtr CreateObject(char const* obj);
};
