//==============================================================================
// Minamoto : Import Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <xxGraphicPlus/xxUtility.h>

class Import
{
public:
    static void Initialize();
    static void Shutdown();
    static xxTexturePtr CreateTexture(char const* img);
    static void MergeNode(xxNodePtr const& target, xxNodePtr const& source, xxNodePtr const& root);
    static void MergeTexture(xxNodePtr const& node);
    static xxNodePtr GetNodeByName(xxNodePtr const& root, std::string const& name);
public:
    static bool EnableAxisUpYToZ;
    static bool EnableMergeNode;
    static bool EnableMergeTexture;
    static bool EnableOptimizeMesh;
    static bool EnableTextureFlipV;
};
