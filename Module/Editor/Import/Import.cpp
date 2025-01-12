//==============================================================================
// Minamoto : Import Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <map>
#include <xxGraphicPlus/xxFile.h>
#include <xxGraphicPlus/xxMaterial.h>
#include <xxGraphicPlus/xxMesh.h>
#include <xxGraphicPlus/xxNode.h>
#include <xxGraphicPlus/xxTexture.h>
#include "Import.h"

//#define STBI_NO_JPEG
//#define STBI_NO_PNG
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_FAILURE_STRINGS
#define STBI_NO_THREAD_LOCALS
#define STB_IMAGE_IMPLEMENTATION
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wunused-function"
#endif
#include <stb/stb_image.h>

#define TAG "Import"

bool Import::EnableAxisUpYToZ = false;
bool Import::EnableMergeNode = false;
bool Import::EnableMergeTexture = true;
bool Import::EnableOptimizeMesh = true;
bool Import::EnableTextureFlipV = true;
//==============================================================================
void Import::Initialize()
{
}
//------------------------------------------------------------------------------
void Import::Shutdown()
{
}
//------------------------------------------------------------------------------
xxTexturePtr Import::CreateTexture(char const* img)
{
    int width = 1;
    int height = 1;
    stbi_uc* uc = stbi_load(img, &width, &height, nullptr, 4);

#if defined(xxWINDOWS)
    uint64_t format = "BGRA8888"_CC;
#else
    uint64_t format = "RGBA8888"_CC;
#endif
    xxTexturePtr texture = xxTexture::Create2D(format, width, height, 1);
    if (texture)
    {
        texture->Name = xxFile::GetName(img);
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
    }

    stbi_image_free(uc);
    return texture;
}
//------------------------------------------------------------------------------
void Import::MergeNode(xxNodePtr const& target, xxNodePtr const& source, xxNodePtr const& root)
{
    std::vector<std::pair<xxNodePtr, xxNodePtr>> merge;
    std::vector<xxNodePtr> append;

    for (xxNodePtr const& right : *source)
    {
        bool found = false;
        for (xxNodePtr const& left : *target)
        {
            if (left->Name == right->Name)
            {
                merge.push_back({ left, right });
                found = true;
                break;
            }
        }
        if (found)
            continue;

        append.push_back(right);
    }

    for (auto [left, right] : merge)
    {
        MergeNode(left, right, root);
        left->Modifiers = right->Modifiers;
    }

    for (auto node : append)
    {
        source->DetachChild(node);
        target->AttachChild(node);
    }

    xxNode::Traversal(target, [&](xxNodePtr const& node)
    {
        for (auto& data : node->Bones)
        {
            xxNodePtr from = data.bone.lock();
            xxNodePtr to;
            if (from)
            {
                to = GetNodeByName(root, from->Name);
            }
            if (to == nullptr)
            {
                to = root;
                xxLog(TAG, "Bone %s is not found", from ? from->Name.c_str() : "(nullptr)");
            }
            data.bone = to;
        }
        return true;
    });
}
//------------------------------------------------------------------------------
void Import::MergeTexture(xxNodePtr const& node)
{
    std::map<std::string, xxTexturePtr> textures;

    xxNode::Traversal(node, [&](xxNodePtr const& node)
    {
        if (node->Material)
        {
            for (xxTexturePtr& texture : node->Material->Textures)
            {
                xxTexturePtr& ref = textures[texture->Name];
                if (ref)
                {
                    texture = ref;
                }
                else
                {
                    ref = texture;
                }
            }
        }
        return true;
    });
}
//------------------------------------------------------------------------------
xxNodePtr Import::GetNodeByName(xxNodePtr const& root, std::string const& name)
{
    xxNodePtr output;

    xxNode::Traversal(root, [&](xxNodePtr const& node)
    {
        if (node->Name == name)
            output = node;
        return output == nullptr;
    });

    return output;
}
//==============================================================================
