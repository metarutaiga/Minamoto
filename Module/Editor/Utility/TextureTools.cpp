//==============================================================================
// Minamoto : TextureTools Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <utility/xxTexture.h>
#include <Runtime/Graphic/Texture.h>
#include "TextureTools.h"

#define STB_DXT_IMPLEMENTATION
#ifdef __clang__
#pragma clang diagnostic ignored "-Wcomma"
#endif
#include <stb/stb_dxt.h>
static void stb__CompressColorBlockEx(unsigned char* dest, unsigned char* block, int mode)
{
    for (int i = 0; i < 16; ++i)
    {
        dest[i * 4 + 3] = 0xFF;
    }
    stb__CompressColorBlock(dest, block, mode);
}
static void stb__ReduceAlphaBlock(unsigned char* dest, unsigned char* src, int stride)
{
    memset(dest, 0, 8);
    for (int i = 0; i < 16; ++i)
    {
        dest[i / 2] |= (src[i * stride] & 0xF0) >> (((i + 1) % 2) * 4);
    }
}
static void stb__CompressAlphaBlockEx(unsigned char* dest, unsigned char* src, int stride)
{
    stb__CompressAlphaBlock(dest, src, stride);
    if (dest[0] == dest[1])
    {
        if (dest[1] == 0xFF)
            memset(dest + 2, 0xFF, 6);
        else if (dest[2] == 0x49)
            memset(dest + 2, 0, 6);
    }
}

//==============================================================================
void TextureTools::CompressTexture(xxTexturePtr const& texture, uint64_t format, std::string const& root, std::string const& subfolder)
{
    if (texture->Format != "RGBA8888"_FOURCC)
        return;
    switch (format)
    {
    case "BC1"_FOURCC:  case "DXT1"_FOURCC:
    case "BC2"_FOURCC:  case "DXT3"_FOURCC:
    case "BC3"_FOURCC:  case "DXT5"_FOURCC:
    case "BC4U"_FOURCC: case "ATI1"_FOURCC:
    case "BC5U"_FOURCC: case "ATI2"_FOURCC:
        break;
    default:
        return;
    }
    xxTexturePtr uncompressed = xxTexture::Create();
    uncompressed->Name = texture->Name;
    uncompressed->Path = texture->Path;
    xxTexture::Reader(uncompressed);
    if ((*uncompressed)() == nullptr)
        return;
    xxTexturePtr compressed = xxTexture::Create(format, uncompressed->Width, uncompressed->Height, uncompressed->Depth, uncompressed->Mipmap, uncompressed->Array);
    for (int array = 0; array < uncompressed->Array; ++array)
    {
        for (int mipmap = 0; mipmap < uncompressed->Mipmap; ++mipmap)
        {
            int levelWidth = (uncompressed->Width >> mipmap);
            int levelHeight = (uncompressed->Height >> mipmap);
            int levelDepth = (uncompressed->Depth >> mipmap);
            if (levelWidth == 0)
                levelWidth = 1;
            if (levelHeight == 0)
                levelHeight = 1;
            if (levelDepth == 0)
                levelDepth = 1;

            void* line = (*compressed)(0, 0, 0, mipmap, array);
            for (int depth = 0; depth < levelDepth; ++depth)
            {
                for (int height = 0; height < levelHeight; height += 4)
                {
                    for (int width = 0; width < levelWidth; width += 4)
                    {
                        unsigned char source[4 * 4 * 4];
                        for (int y = 0; y < 4; ++y)
                        {
                            for (int x = 0; x < 4; ++x)
                            {
                                int offsetX = std::min(width + x, levelWidth - 1);
                                int offsetY = std::min(height + y, levelHeight - 1);
                                memcpy(source + y * 4 * 4 + x * 4, (*uncompressed)(offsetX, offsetY, depth, mipmap, array), 4);
                            }
                        }
                        switch (format)
                        {
                        case "BC1"_FOURCC:
                        case "DXT1"_FOURCC:
                            stb__CompressColorBlockEx((unsigned char*)line, source, STB_DXT_HIGHQUAL);
                            line = (char*)line + 8;
                            break;
                        case "BC2"_FOURCC:
                        case "DXT3"_FOURCC:
                            stb__ReduceAlphaBlock((unsigned char*)line, source + 3, 4);
                            stb__CompressColorBlockEx((unsigned char*)line + 8, source, STB_DXT_HIGHQUAL);
                            line = (char*)line + 16;
                            break;
                        case "BC3"_FOURCC:
                        case "DXT5"_FOURCC:
                            stb__CompressAlphaBlockEx((unsigned char*)line, source + 3, 4);
                            stb__CompressColorBlockEx((unsigned char*)line + 8, source, STB_DXT_HIGHQUAL);
                            line = (char*)line + 16;
                            break;
                        case "BC4U"_FOURCC:
                        case "ATI1"_FOURCC:
                            stb__CompressAlphaBlockEx((unsigned char*)line, source, 4);
                            line = (char*)line + 8;
                            break;
                        case "BC5U"_FOURCC:
                        case "ATI2"_FOURCC:
                            stb__CompressAlphaBlockEx((unsigned char*)line, source, 4);
                            stb__CompressAlphaBlockEx((unsigned char*)line + 8, source + 1, 4);
                            line = (char*)line + 16;
                            break;
                        }
                    }
                }
            }
        }
    }
    std::string ext = ".unknown";
    switch (format)
    {
    case "BC1"_FOURCC:  case "DXT1"_FOURCC: ext = ".bc1";   break;
    case "BC2"_FOURCC:  case "DXT3"_FOURCC: ext = ".bc2";   break;
    case "BC3"_FOURCC:  case "DXT5"_FOURCC: ext = ".bc3";   break;
    case "BC4U"_FOURCC: case "ATI1"_FOURCC: ext = ".bc4u";  break;
    case "BC5U"_FOURCC: case "ATI2"_FOURCC: ext = ".bc5u";  break;
    }
    Texture::DDSWriter(compressed, root + subfolder + uncompressed->Name + ext);
}
//==============================================================================
