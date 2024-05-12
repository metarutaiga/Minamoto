//==============================================================================
// Minamoto : Texture Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <xxGraphic.h>
#include <map>
#include <utility/xxFile.h>
#include <utility/xxImage.h>
#include "Texture.h"

#define STBI_ONLY_PNG
#define STBI_NO_FAILURE_STRINGS
#define STB_IMAGE_IMPLEMENTATION
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunused-function"
#endif
#include <stb/stb_image.h>

//==============================================================================
static std::map<std::string, xxImagePtr> images;
//------------------------------------------------------------------------------
void Texture::Initialize()
{
    xxImage::Calculate = Texture::Calculate;
    xxImage::Loader = Texture::Loader;
}
//------------------------------------------------------------------------------
void Texture::Shutdown()
{
    images.clear();
}
//------------------------------------------------------------------------------
size_t Texture::Calculate(uint64_t format, int width, int height, int depth)
{
    switch (format)
    {
    case "BC1 "_FOURCC:
    case "BC4 "_FOURCC:
    case "DXT1"_FOURCC:
        width = (width + 3) / 4;
        height = (height + 3) / 4;
        return width * height * depth * 8;
    case "BC2 "_FOURCC:
    case "BC3 "_FOURCC:
    case "BC5 "_FOURCC:
    case "BC6H"_FOURCC:
    case "BC7 "_FOURCC:
    case "DXT3"_FOURCC:
    case "DXT5"_FOURCC:
        width = (width + 3) / 4;
        height = (height + 3) / 4;
        return width * height * depth * 16;
    }
    uint8_t bits = 0;
    for (int i = 0; i < 4; ++i)
    {
        uint8_t bit = uint8_t(format >> (i * 8 + 32));
        if (bit == 0)
            bit = '8';
        bits += bit - '0';
    }
    size_t bytes = (bits + 7) / 8;
    return width * height * depth * bytes;
}
//------------------------------------------------------------------------------
void Texture::Loader(xxImagePtr& image, std::string const& path)
{
    if (image == nullptr || (*image)() != nullptr)
        return;

    auto& ref = images[image->Name];
    if (ref != nullptr)
    {
        image = ref;
        return;
    }
    ref = image;

    Reader(image, path);
}
//------------------------------------------------------------------------------
void Texture::Reader(xxImagePtr& image, std::string const& path)
{
    if (image == nullptr || (*image)() != nullptr)
        return;

    std::string filename = path + image->Name;
    if (strcasestr(image->Name.c_str(), ".dds"))
    {
        DDSReader(image, filename);
    }
    else if (strcasestr(image->Name.c_str(), ".png"))
    {
        PNGReader(image, filename);
    }
    if ((*image)() == nullptr)
    {
        image->Initialize("RGBA8888"_FOURCC, 1, 1, 1, 1, 1);
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
#include <xxGraphic/dxsdk/ddraw.h>
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
void Texture::DDSReader(xxImagePtr& image, std::string const& filename)
{
    if (image == nullptr || (*image)() != nullptr)
        return;

    xxFile* file = xxFile::Load(filename.c_str());
    xxLocalBreak()
    {
        if (file == nullptr)
            break;
        DDS_HEADER header;
        if (file->Read(&header, sizeof(DDS_HEADER)) == false)
            break;
        if (header.dwMagic != "DDS "_FOURCC)
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
            char temp[8] = {};
            if (header.ddspf.dwFlags & DDPF_FOURCC)
            {
                memcpy(temp, &header.ddspf.dwFourCC, 4);
            }
            if (header.ddspf.dwFlags & DDPF_RGB)
            {
                if (header.ddspf.dwRBitMask < header.ddspf.dwBBitMask)
                {
                    temp[0] = 'R';
                    temp[1] = 'G';
                    temp[2] = 'B';
                    temp[4] = xxPopulationCount(header.ddspf.dwRBitMask);
                    temp[5] = xxPopulationCount(header.ddspf.dwGBitMask);
                    temp[6] = xxPopulationCount(header.ddspf.dwBBitMask);
                }
                else
                {
                    temp[0] = 'B';
                    temp[1] = 'G';
                    temp[2] = 'R';
                    temp[4] = xxPopulationCount(header.ddspf.dwBBitMask);
                    temp[5] = xxPopulationCount(header.ddspf.dwGBitMask);
                    temp[6] = xxPopulationCount(header.ddspf.dwRBitMask);
                }
                if (header.ddspf.dwFlags & DDPF_ALPHAPIXELS)
                {
                    temp[3] = 'A';
                    temp[7] = xxPopulationCount(header.ddspf.dwABitMask);
                }
            }
            format = *(uint64_t*)temp;
        }
        image->Initialize(format, width, height, depth, mipmap, 1);
        for (int m = 0; m < mipmap; ++m)
        {
            if (width == 0)
                width = 1;
            if (height == 0)
                height = 1;
            if (depth == 0)
                depth = 1;

            void* data = (*image)(0, 0, 0, m);
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
void Texture::DDSWriter(xxImagePtr& image, std::string const& filename)
{
    if (image == nullptr || (*image)() == nullptr)
        return;

    xxFile* file = xxFile::Load(filename.c_str());
    xxLocalBreak()
    {
        if (file == nullptr)
            break;
        DDS_HEADER header = {};
        header.dwMagic = "DDS "_FOURCC;
        header.dwSize = sizeof(DDS_HEADER) - sizeof(uint32_t);
        header.dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
        header.dwHeight = image->Height;
        header.dwWidth = image->Width;
        header.dwPitchOrLinearSize = 0;
        header.dwDepth = image->Depth;
        header.dwMipMapCount = image->Mipmap;
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
        switch (image->Format)
        {
        default:
            for (int i = 0; i < 4; ++i)
            {
                uint8_t channel = uint8_t(image->Format >> (i * 8));
                uint8_t bits = uint8_t(image->Format >> (i * 8 + 32)) - '0';
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
        case "BC1 "_FOURCC:
        case "BC2 "_FOURCC:
        case "BC3 "_FOURCC:
        case "BC4 "_FOURCC:
        case "BC5 "_FOURCC:
        case "BC6H"_FOURCC:
        case "BC7 "_FOURCC:
        case "DXT1"_FOURCC:
        case "DXT3"_FOURCC:
        case "DXT5"_FOURCC:
            header.ddspf.dwFlags |= DDPF_FOURCC;
            header.ddspf.dwFourCC = uint32_t(image->Format);
            break;
        }
        file->Write(&header, sizeof(DDS_HEADER));
        int width = 1;
        int height = 1;
        int depth = 1;
        int mipmap = 1;
        for (int m = 0; m < mipmap; ++m)
        {
            if (width == 0)
                width = 1;
            if (height == 0)
                height = 1;
            if (depth == 0)
                depth = 1;

            void* data = (*image)(0, 0, 0, m);
            size_t size = Calculate(image->Format, width, height, depth);
            file->Write(data, size);

            width >>= 1;
            height >>= 1;
            depth >>= 1;
        }
    }
    delete file;
}
//------------------------------------------------------------------------------
void Texture::PNGReader(xxImagePtr& image, std::string const& filename)
{
    if (image == nullptr || (*image)() != nullptr)
        return;

#if defined(xxWINDOWS)
    uint64_t format = *(uint64_t*)"BGRA8888";
#else
    uint64_t format = *(uint64_t*)"RGBA8888";
#endif
    int width = 1;
    int height = 1;
    stbi_uc* uc = stbi_load(filename.c_str(), &width, &height, nullptr, 4);

    image->Initialize(format, width, height, 1, 1, 1);
    if (uc)
    {
#if defined(xxWINDOWS)
        for (int i = 0; i < width * height * 4; i += 4)
        {
            std::swap(uc[i + 0], uc[i + 2]);
        }
#endif
        memcpy((*image)(), uc, width * height * 4);
    }

    stbi_image_free(uc);
}
//==============================================================================
