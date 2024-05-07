//==============================================================================
// Minamoto : Texture Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <xxGraphic.h>
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
void Texture::Initialize()
{
    xxImage::Loader = [](xxImage& image, std::string const& path)
    {
        if (image() != nullptr)
            return;
        struct xxImageInternal : public xxImage { using xxImage::Initialize; };
        xxImageInternal* internal = reinterpret_cast<xxImageInternal*>(&image);

#if defined(xxWINDOWS)
        uint64_t format = *(uint64_t*)"BGRA8888";
#else
        uint64_t format = *(uint64_t*)"RGBA8888";
#endif
        int width = 1;
        int height = 1;
        std::string filename = path + image.Name;
        stbi_uc* uc = stbi_load(filename.c_str(), &width, &height, nullptr, 4);

        internal->Initialize(format, width, height, 1, 1, 1);
        if (uc)
        {
#if defined(xxWINDOWS)
            for (int i = 0; i < width * height * 4; i += 4)
            {
                std::swap(uc[i + 0], uc[i + 2]);
            }
#endif
            memcpy(image(), uc, width * height * 4);
        }

        stbi_image_free(uc);
    };
}
//------------------------------------------------------------------------------
void Texture::Shutdown()
{
}
//==============================================================================
