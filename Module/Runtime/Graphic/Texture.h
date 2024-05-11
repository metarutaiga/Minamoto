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
    static void Loader(xxImagePtr& image, std::string const& path);
    static void Reader(xxImagePtr& image, std::string const& path);
    static void DDSReader(xxImagePtr& image, std::string const& filename);
    static void DDSWriter(xxImagePtr& image, std::string const& filename);
    static void PNGReader(xxImagePtr& image, std::string const& filename);
};

constexpr uint64_t operator""_FOURCC (char const* text, size_t length)
{
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i)
    {
        value += (uint8_t)text[i] << (i * 8);
    }
    return value;
};
