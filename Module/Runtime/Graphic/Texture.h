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
    static void Reader(xxTexturePtr& texture, std::string const& path);
    static void DDSReader(xxTexturePtr& texture, std::string const& filename);
    static void DDSWriter(xxTexturePtr& texture, std::string const& filename);
    static void PNGReader(xxTexturePtr& texture, std::string const& filename);
};

constexpr uint64_t operator""_FOURCC (char const* text, size_t length)
{
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i)
    {
        value += uint64_t(uint8_t(text[i])) << (i * 8);
    }
    return value;
};
