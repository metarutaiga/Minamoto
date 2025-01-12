//==============================================================================
// Minamoto : Texture Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Runtime.h"

struct RuntimeAPI Texture
{
    static void Initialize();
    static void Shutdown();
    static size_t Calculate(uint64_t format, int width, int height, int depth);
    static void Loader(xxTexturePtr& texture, std::string const& path);
    static void Reader(xxTexturePtr const& texture);
    static void DDSReader(xxTexturePtr const& texture, std::string const& filename);
    static void DDSWriter(xxTexturePtr const& texture, std::string const& filename);
    static void STBReader(xxTexturePtr const& texture, std::string const& filename);
};
