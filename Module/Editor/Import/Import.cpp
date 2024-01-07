//==============================================================================
// Minamoto : Import Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxImage.h>
#include <utility/xxMesh.h>
#include "Import.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#pragma clang diagnostic ignored "-Wunused-function"
#include <stb/stb_image.h>

//==============================================================================
void Import::Initialize()
{
    xxImage::ImageLoader = [](xxImagePtr const& image, std::string const& path)
    {
        if (image == nullptr || (*image)() != nullptr)
            return;

        std::string filename = path + image->Name;

        int width = 0;
        int height = 0;
        stbi_uc* uc = stbi_load(filename.c_str(), &width, &height, nullptr, 4);
        if (uc == nullptr)
            return;

        const_cast<int&>(image->Width) = width;
        const_cast<int&>(image->Height) = height;
        const_cast<int&>(image->Depth) = 1;
        const_cast<int&>(image->Mipmap) = 1;
        const_cast<int&>(image->Array) = 1;
        image->Initialize();
        memcpy((*image)(), uc, width * height * 4);

        stbi_image_free(uc);
    };
}
//------------------------------------------------------------------------------
void Import::Shutdown()
{
    
}
//------------------------------------------------------------------------------
xxImagePtr Import::CreateImage(char const* img)
{
    int width = 0;
    int height = 0;
    stbi_uc* uc = stbi_load(img, &width, &height, nullptr, 4);
    if (uc == nullptr)
        return nullptr;

    xxImagePtr image = xxImage::Create2D(0, width, height, 1);
    if (image)
    {
        char const* name = strrchr(img, '/');
        if (name == nullptr)
            name = strrchr(img, '\\');
        if (name == nullptr)
            name = img - 1;
        name += 1;
        image->Name = name;
        memcpy((*image)(), uc, width * height * 4);
    }

    stbi_image_free(uc);
    return image;
}
//------------------------------------------------------------------------------
xxMeshPtr Import::CreateMesh(std::vector<xxVector3> const& vertices, std::vector<xxVector3> const& normals, std::vector<xxVector4> const& colors, std::vector<xxVector2> const& textures)
{
    if (vertices.empty())
        return nullptr;

    int normal = int(normals.size() / vertices.size());
    int color = int(colors.size() / vertices.size());
    int texture = int(textures.size() / vertices.size());
    xxMeshPtr mesh = xxMesh::Create(normal, color, texture);
    if (mesh == nullptr)
        return nullptr;

    mesh->SetVertexCount(int(vertices.size()));

    auto source = vertices.begin();
    auto target = mesh->GetVertex();
    while (source != vertices.end() && target.isEnd() == false)
    {
        (*target++) = (*source++);
    }

    if (normals.size() == vertices.size())
    {
        auto source = normals.begin();
        auto target = mesh->GetNormal(0);
        while (source != normals.end() && target.isEnd() == false)
        {
            (*target++) = (*source++);
        }
    }

    if (colors.size() == vertices.size())
    {
        auto source = colors.begin();
        auto target = mesh->GetColor(0);
        while (source != colors.end() && target.isEnd() == false)
        {
            (*target++) = (*source++).ToInteger();
        }
    }

    if (textures.size() == vertices.size())
    {
        auto source = textures.begin();
        auto target = mesh->GetTexture(0);
        while (source != textures.end() && target.isEnd() == false)
        {
            (*target++) = (*source++);
        }
    }

    return mesh;
}
//==============================================================================
