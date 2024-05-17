//==============================================================================
// Minamoto : Texture Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <algorithm>
#include <map>
#include <xxGraphicPlus/xxFile.h>
#include <xxGraphicPlus/xxTexture.h>
#include "Texture.h"

#define STBI_ONLY_PNG
#define STBI_NO_FAILURE_STRINGS
#define STB_IMAGE_IMPLEMENTATION
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunused-function"
#endif
#include <stb/stb_image.h>

//==============================================================================
static std::map<std::string, xxTexturePtr> textures;
//------------------------------------------------------------------------------
void Texture::Initialize()
{
    xxTexture::Calculate = Texture::Calculate;
    xxTexture::Loader = Texture::Loader;
    xxTexture::Reader = Texture::Reader;
}
//------------------------------------------------------------------------------
void Texture::Shutdown()
{
    textures.clear();
}
//------------------------------------------------------------------------------
size_t Texture::Calculate(uint64_t format, int width, int height, int depth)
{
    switch (format)
    {
    case "BC1"_cc:
    case "BC4S"_cc:
    case "BC4U"_cc:
    case "DXT1"_cc:
    case "ATI1"_cc:
        width = (width + 3) / 4;
        height = (height + 3) / 4;
        return width * height * depth * 8;
    case "BC2"_cc:
    case "BC3"_cc:
    case "BC5S"_cc:
    case "BC5U"_cc:
    case "BC6H"_cc:
    case "BC7"_cc:
    case "DXT3"_cc:
    case "DXT5"_cc:
    case "ATI2"_cc:
        width = (width + 3) / 4;
        height = (height + 3) / 4;
        return width * height * depth * 16;
    }
    size_t channels = 0;
    size_t bits = 0;
    for (int i = 0; i < 8; ++i)
    {
        uint8_t bit = uint8_t(format >> (i * 8));
        if (bit >= 'A' && bit <= 'Z')
            channels++;
        if (bit >= '0' && bit <= '9')
            bits += bit - '0';
    }
    if (bits == 0)
        bits = channels * 8;
    size_t bytes = (bits + 7) / 8;
    return width * height * depth * bytes;
}
//------------------------------------------------------------------------------
void Texture::Loader(xxTexturePtr& texture, std::string const& path)
{
    if (texture == nullptr || (*texture)() != nullptr)
        return;

    auto& ref = textures[texture->Name];
    if (ref != nullptr)
    {
        texture = ref;
        return;
    }
    ref = texture;

    texture->Path = path;
}
//------------------------------------------------------------------------------
void Texture::Reader(xxTexturePtr const& texture)
{
    if (texture == nullptr || (*texture)() != nullptr)
        return;

    std::string filename = texture->Path + texture->Name;
    if (strcasestr(texture->Name.c_str(), ".dds"))
    {
        DDSReader(texture, filename);
    }
    else if (strcasestr(texture->Name.c_str(), ".png"))
    {
        PNGReader(texture, filename);
    }
    if ((*texture)() == nullptr)
    {
        texture->Initialize("RGBA8888"_CC, 1, 1, 1, 1, 1);
    }
}
//------------------------------------------------------------------------------
#if defined(xxANDROID) || defined(xxMACOS) || defined(xxIOS)
#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t
#define LONG uint32_t
#define GUID struct { uint32_t a; uint16_t b; uint16_t c; uint8_t d[8]; }
#define HWND void*
#define LPRECT void*
#define LPVOID void*
#define FAR
#define PASCAL
#define _Return_type_success_(...)
#endif
#include <dxsdk/ddraw.h>
struct DDS_HEADER
{
    uint32_t        dwMagic;
    uint32_t        dwSize;
    uint32_t        dwFlags;
    uint32_t        dwHeight;
    uint32_t        dwWidth;
    uint32_t        dwPitchOrLinearSize;
    uint32_t        dwDepth;
    uint32_t        dwMipMapCount;
    uint32_t        dwReserved1[11];
    struct DDS_PIXELFORMAT
    {
        uint32_t    dwSize;
        uint32_t    dwFlags;
        uint32_t    dwFourCC;
        uint32_t    dwRGBBitCount;
        uint32_t    dwRBitMask;
        uint32_t    dwGBitMask;
        uint32_t    dwBBitMask;
        uint32_t    dwABitMask;
    } ddspf;
    uint32_t        dwCaps;
    uint32_t        dwCaps2;
    uint32_t        dwCaps3;
    uint32_t        dwCaps4;
    uint32_t        dwReserved2;
};
static_assert(sizeof(DDS_HEADER) == 128);
//------------------------------------------------------------------------------
void Texture::DDSReader(xxTexturePtr const& texture, std::string const& filename)
{
    if (texture == nullptr || (*texture)() != nullptr)
        return;

    xxFile* file = xxFile::Load(filename.c_str());
    xxLocalBreak()
    {
        if (file == nullptr)
            break;
        DDS_HEADER header;
        if (file->Read(&header, sizeof(DDS_HEADER)) != sizeof(DDS_HEADER))
            break;
        if (header.dwMagic != "DDS "_cc)
            break;
        if ((header.dwFlags & (DDSD_CAPS | DDSD_PIXELFORMAT)) == 0)
            break;
        if ((header.dwCaps & DDSCAPS_TEXTURE) == 0)
            break;
        uint64_t format = 0;
        int width = 1;
        int height = 1;
        int depth = 1;
        int mipmap = 1;
        if (header.dwFlags & DDSD_WIDTH)
            width = header.dwWidth;
        if (header.dwFlags & DDSD_HEIGHT)
            height = header.dwHeight;
        if (header.dwFlags & DDSD_DEPTH && header.dwCaps2 & DDSCAPS2_VOLUME)
            depth = header.dwDepth;
        if (header.dwFlags & DDSD_MIPMAPCOUNT && header.dwCaps & (DDSCAPS_COMPLEX | DDSCAPS_MIPMAP))
            mipmap = header.dwMipMapCount;
        if (header.dwFlags & DDSD_PIXELFORMAT && header.ddspf.dwSize == sizeof(DDS_HEADER::DDS_PIXELFORMAT))
        {
            if (header.ddspf.dwFlags & DDPF_FOURCC)
            {
                memcpy(&format, &header.ddspf.dwFourCC, 4);
            }
            if (header.ddspf.dwFlags & DDPF_RGB)
            {
                uint64_t channels[4];
                uint64_t* channelsEnd = channels;
                if (header.ddspf.dwRBitMask) (*channelsEnd++) = 'R' | uint64_t(header.ddspf.dwRBitMask) << 32;
                if (header.ddspf.dwGBitMask) (*channelsEnd++) = 'G' | uint64_t(header.ddspf.dwGBitMask) << 32;
                if (header.ddspf.dwBBitMask) (*channelsEnd++) = 'B' | uint64_t(header.ddspf.dwBBitMask) << 32;
                if (header.ddspf.dwABitMask) (*channelsEnd++) = 'A' | uint64_t(header.ddspf.dwABitMask) << 32;
                std::sort(channels, channelsEnd);

                char* component = (char*)&format;
                for (uint64_t* channel = channels; channel != channelsEnd; ++channel)
                {
                    (*component++) = char(*channel);
                }
                for (uint64_t* channel = channels; channel != channelsEnd; ++channel)
                {
                    (*component++) = '0' + xxPopulationCount(*channel >> 32);
                }
            }
        }
        texture->Initialize(format, width, height, depth, mipmap, 1);
        for (int m = 0; m < mipmap; ++m)
        {
            if (width == 0)
                width = 1;
            if (height == 0)
                height = 1;
            if (depth == 0)
                depth = 1;

            void* data = (*texture)(0, 0, 0, m);
            size_t size = Calculate(format, width, height, depth);
            file->Read(data, size);

            width >>= 1;
            height >>= 1;
            depth >>= 1;
        }
    }
    delete file;
}
//------------------------------------------------------------------------------
void Texture::DDSWriter(xxTexturePtr const& texture, std::string const& filename)
{
    if (texture == nullptr || (*texture)() == nullptr)
        return;

    xxFile* file = xxFile::Save(filename.c_str());
    xxLocalBreak()
    {
        if (file == nullptr)
            break;
        DDS_HEADER header = {};
        header.dwMagic = uint32_t("DDS "_cc);
        header.dwSize = sizeof(DDS_HEADER) - sizeof(header.dwMagic);
        header.dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
        header.dwHeight = texture->Height;
        header.dwWidth = texture->Width;
        header.dwPitchOrLinearSize = 0;
        header.dwDepth = texture->Depth;
        header.dwMipMapCount = texture->Mipmap;
        header.dwCaps = DDSCAPS_TEXTURE;
        if (header.dwHeight > 0)
            header.dwFlags |= DDSD_HEIGHT;
        if (header.dwWidth > 0)
            header.dwFlags |= DDSD_WIDTH;
        if (header.dwDepth > 1)
        {
            header.dwFlags |= DDSD_DEPTH;
            header.dwCaps2 |= DDSCAPS2_VOLUME;
        }
        if (header.dwMipMapCount > 1)
        {
            header.dwFlags |= DDSD_MIPMAPCOUNT;
            header.dwCaps |= (DDSCAPS_COMPLEX | DDSCAPS_MIPMAP);
        }
        header.ddspf.dwSize = sizeof(DDS_HEADER::DDS_PIXELFORMAT);
        switch (texture->Format)
        {
        default:
            for (int i = 0; i < 4; ++i)
            {
                uint8_t channel = uint8_t(texture->Format >> (i * 8));
                uint8_t bits = uint8_t(texture->Format >> (i * 8 + 32)) - '0';
                switch (channel)
                {
                case 'R':
                    header.ddspf.dwFlags |= DDPF_RGB;
                    header.ddspf.dwRBitMask = ((1 << (bits + 1)) - 1) << header.ddspf.dwRGBBitCount;
                    header.ddspf.dwRGBBitCount += bits;
                    break;
                case 'G':
                    header.ddspf.dwFlags |= DDPF_RGB;
                    header.ddspf.dwGBitMask = ((1 << (bits + 1)) - 1) << header.ddspf.dwRGBBitCount;
                    header.ddspf.dwRGBBitCount += bits;
                    break;
                case 'B':
                    header.ddspf.dwFlags |= DDPF_RGB;
                    header.ddspf.dwBBitMask = ((1 << (bits + 1)) - 1) << header.ddspf.dwRGBBitCount;
                    header.ddspf.dwRGBBitCount += bits;
                    break;
                case 'A':
                    header.ddspf.dwFlags |= DDPF_ALPHAPIXELS;
                    header.ddspf.dwABitMask = ((1 << (bits + 1)) - 1) << header.ddspf.dwRGBBitCount;
                    header.ddspf.dwRGBBitCount += bits;
                    break;
                }
            }
            break;
        case "BC1"_cc:
        case "BC2"_cc:
        case "BC3"_cc:
        case "BC4S"_cc:
        case "BC4U"_cc:
        case "BC5S"_cc:
        case "BC5U"_cc:
        case "BC6H"_cc:
        case "BC7"_cc:
        case "DXT1"_cc:
        case "DXT3"_cc:
        case "DXT5"_cc:
        case "ATI1"_cc:
        case "ATI2"_cc:
            header.ddspf.dwFlags |= DDPF_FOURCC;
            header.ddspf.dwFourCC = uint32_t(texture->Format);
            break;
        }
        file->Write(&header, sizeof(DDS_HEADER));
        int width = texture->Width;
        int height = texture->Height;
        int depth = texture->Depth;
        int mipmap = texture->Mipmap;
        for (int m = 0; m < mipmap; ++m)
        {
            if (width == 0)
                width = 1;
            if (height == 0)
                height = 1;
            if (depth == 0)
                depth = 1;

            void* data = (*texture)(0, 0, 0, m);
            size_t size = Calculate(texture->Format, width, height, depth);
            file->Write(data, size);

            width >>= 1;
            height >>= 1;
            depth >>= 1;
        }
    }
    delete file;
}
//------------------------------------------------------------------------------
void Texture::PNGReader(xxTexturePtr const& texture, std::string const& filename)
{
    if (texture == nullptr || (*texture)() != nullptr)
        return;

#if defined(xxWINDOWS)
    uint64_t format = "BGRA8888"_CC;
#else
    uint64_t format = "RGBA8888"_CC;
#endif
    int width = 1;
    int height = 1;
    stbi_uc* uc = stbi_load(filename.c_str(), &width, &height, nullptr, 4);

    texture->Initialize(format, width, height, 1, 1, 1);
    if (uc)
    {
#if defined(xxWINDOWS)
        for (int i = 0; i < width * height * 4; i += 4)
        {
            std::swap(uc[i + 0], uc[i + 2]);
        }
#endif
        memcpy((*texture)(), uc, width * height * 4);
    }

    stbi_image_free(uc);
}
//==============================================================================
