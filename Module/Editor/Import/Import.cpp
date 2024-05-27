//==============================================================================
// Minamoto : Import Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <map>
#include <xxGraphicPlus/xxMaterial.h>
#include <xxGraphicPlus/xxMesh.h>
#include <xxGraphicPlus/xxNode.h>
#include <xxGraphicPlus/xxTexture.h>
#include <Runtime/Graphic/Texture.h>
#include "Import.h"

#define STBI_ONLY_PNG
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
        char const* name = strrchr(img, '/');
        if (name == nullptr)
            name = strrchr(img, '\\');
        if (name == nullptr)
            name = img - 1;
        name += 1;
        texture->Name = name;
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
xxMeshPtr Import::CreateMesh(std::vector<xxVector3> const& vertices, std::vector<xxVector3> const& normals, std::vector<xxVector4> const& colors, std::vector<xxVector2> const& textures)
{
    if (vertices.empty())
        return nullptr;

    int normal = int(normals.size() / vertices.size());
    int color = int(colors.size() / vertices.size());
    int texture = int(textures.size() / vertices.size());
    xxMeshPtr mesh = xxMesh::Create(false, normal, color, texture);
    if (mesh == nullptr)
        return nullptr;

    mesh->SetVertexCount(int(vertices.size()));

    auto source = vertices.begin();
    for (auto& vertex : mesh->GetVertex())
    {
        vertex = (*source++);
    }

    if (normals.size() == vertices.size())
    {
        auto source = normals.begin();
        for (auto& normal : mesh->GetNormal(0))
        {
            normal = (*source++);
        }
    }

    if (colors.size() == vertices.size())
    {
        auto source = colors.begin();
        for (auto& color : mesh->GetColor(0))
        {
            color = (*source++).ToInteger();
        }
    }

    if (textures.size() == vertices.size())
    {
        auto source = textures.begin();
        for (auto& texture : mesh->GetTexture(0))
        {
            texture = (*source++);
        }
    }

    mesh->CalculateBound();
    return mesh;
}
//------------------------------------------------------------------------------
xxMeshPtr Import::OptimizeMesh(xxMeshPtr const& mesh)
{
    if (mesh == nullptr)
        return nullptr;
    if (mesh->Index)
        return mesh;

    bool skinning = mesh->Skinning;
    int vertexCount = mesh->VertexCount;
    int normalCount = mesh->NormalCount;
    int colorCount = mesh->ColorCount;
    int textureCount = mesh->TextureCount;
    if (normalCount > 8 || colorCount > 8 || textureCount > 8)
    {
        xxLog(TAG, "OptimizeMesh failed (%d,%d,%d)", normalCount, colorCount, textureCount);
        return mesh;
    }

    float begin = xxGetCurrentTime();

    xxStrideIterator<xxVector3> inputVertices = mesh->GetVertex();
    xxStrideIterator<xxVector3> inputBoneWeight = mesh->GetBoneWeight();
    xxStrideIterator<uint32_t> inputBoneIndices = mesh->GetBoneIndices();
    xxStrideIterator<xxVector3> inputNormals[8] =
    {
        mesh->GetNormal(0), mesh->GetNormal(1), mesh->GetNormal(2), mesh->GetNormal(3),
        mesh->GetNormal(4), mesh->GetNormal(5), mesh->GetNormal(6), mesh->GetNormal(7),
    };
    xxStrideIterator<uint32_t> inputColors[8] =
    {
        mesh->GetColor(0), mesh->GetColor(1), mesh->GetColor(2), mesh->GetColor(3),
        mesh->GetColor(4), mesh->GetColor(5), mesh->GetColor(6), mesh->GetColor(7),
    };
    xxStrideIterator<xxVector2> inputTextures[8] =
    {
        mesh->GetTexture(0), mesh->GetTexture(1), mesh->GetTexture(2), mesh->GetTexture(3),
        mesh->GetTexture(4), mesh->GetTexture(5), mesh->GetTexture(6), mesh->GetTexture(7),
    };

    std::vector<xxVector3> vertices;
    std::vector<xxVector3> boneWeights;
    std::vector<uint32_t> boneIndices;
    std::vector<xxVector3> normals;
    std::vector<uint32_t> colors;
    std::vector<xxVector2> textures;
    std::vector<uint16_t> indices;

    for (int i = 0; i < vertexCount; ++i)
    {
        vertices.push_back(*inputVertices++);
        if (skinning)
        {
            boneWeights.push_back(*inputBoneWeight++);
            boneIndices.push_back(*inputBoneIndices++);
        }
        for (int j = 0; j < normalCount; ++j)
            normals.push_back(*inputNormals[j]++);
        for (int j = 0; j < colorCount; ++j)
            colors.push_back(*inputColors[j]++);
        for (int j = 0; j < textureCount; ++j)
            textures.push_back(*inputTextures[j]++);
        indices.push_back(i);
    }

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        auto compare = [](auto& container, size_t a, size_t b, int c)
        {
            for (int i = 0; i < c; ++i)
                if (container[a * c + i] != container[b * c + i])
                    return false;
            return true;
        };

        auto remove = [](auto& container, size_t a, int c)
        {
            for (int i = 0; i < c; ++i)
                container[a * c + i] = container[container.size() - c + i];
            container.pop_back();
        };

        auto replace = [](auto& container, size_t from, size_t to)
        {
            for (size_t i = std::min(from, to); i < container.size(); ++i)
                if (container[i] == from)
                    container[i] = decltype(container[i])(to);
        };

        for (size_t j = i + 1; j < vertices.size(); ++j)
        {
            if (compare(vertices, i, j, 1) == false)
                continue;
            if (compare(boneWeights, i, j, skinning ? 1 : 0) == false)
                continue;
            if (compare(boneIndices, i, j, skinning ? 1 : 0) == false)
                continue;
            if (compare(normals, i, j, normalCount) == false)
                continue;
            if (compare(colors, i, j, colorCount) == false)
                continue;
            if (compare(textures, i, j, textureCount) == false)
                continue;
            remove(vertices, j, 1);
            remove(boneWeights, j, skinning ? 1 : 0);
            remove(boneIndices, j, skinning ? 1 : 0);
            remove(normals, j, normalCount);
            remove(colors, j, colorCount);
            remove(textures, j, textureCount);
            replace(indices, j, i);
            replace(indices, vertices.size(), j);
            --j;
        }
    }

    xxMeshPtr output = xxMesh::Create(skinning, normalCount, colorCount, textureCount);
    output->Name = mesh->Name;
    output->SetVertexCount((int)vertices.size());
    output->SetIndexCount((int)indices.size());

    xxStrideIterator<xxVector3> outputVertices = output->GetVertex();
    xxStrideIterator<xxVector3> outputBoneWeight = output->GetBoneWeight();
    xxStrideIterator<uint32_t> outputBoneIndices = output->GetBoneIndices();
    xxStrideIterator<xxVector3> outputNormals[8] =
    {
        output->GetNormal(0), output->GetNormal(1), output->GetNormal(2), output->GetNormal(3),
        output->GetNormal(4), output->GetNormal(5), output->GetNormal(6), output->GetNormal(7),
    };
    xxStrideIterator<uint32_t> outputColors[8] =
    {
        output->GetColor(0), output->GetColor(1), output->GetColor(2), output->GetColor(3),
        output->GetColor(4), output->GetColor(5), output->GetColor(6), output->GetColor(7),
    };
    xxStrideIterator<xxVector2> outputTextures[8] =
    {
        output->GetTexture(0), output->GetTexture(1), output->GetTexture(2), output->GetTexture(3),
        output->GetTexture(4), output->GetTexture(5), output->GetTexture(6), output->GetTexture(7),
    };

    for (size_t i = 0; i < vertices.size(); ++i)
    {
        (*outputVertices++) = vertices[i];
        if (skinning)
        {
            (*outputBoneWeight++) = boneWeights[i];
            (*outputBoneIndices++) = boneIndices[i];
        }
        for (int j = 0; j < normalCount; ++j)
            (*outputNormals[j]++) = normals[i * normalCount + j];
        for (int j = 0; j < colorCount; ++j)
            (*outputColors[j]++) = colors[i * colorCount + j];
        for (int j = 0; j < textureCount; ++j)
            (*outputTextures[j]++) = textures[i * textureCount + j];
    }
    output->CalculateBound();

    uint16_t* outputIndices = output->Index;

    for (size_t i = 0; i < indices.size(); ++i)
        (*outputIndices++) = indices[i];

    float time = xxGetCurrentTime() - begin;

    xxLog(TAG, "OptimizeMesh : %s Vertex count from %d to %d and index count %d (%.0fus)", mesh->Name.c_str(), mesh->VertexCount, output->VertexCount, output->IndexCount, time * 1000000);

    return output;
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
