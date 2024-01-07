//==============================================================================
// Minamoto : Import Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <utility/xxUtility.h>

class Import
{
public:
    static void Initialize();
    static void Shutdown();
    static xxImagePtr CreateImage(char const* img);
    static xxMeshPtr CreateMesh(std::vector<xxVector3> const& vertices, std::vector<xxVector3> const& normals, std::vector<xxVector4> const& colors, std::vector<xxVector2> const& textures);
};
