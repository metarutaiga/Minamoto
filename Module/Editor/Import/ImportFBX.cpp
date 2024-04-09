//==============================================================================
// Minamoto : ImportFBX Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <utility/xxFile.h>
#include <utility/xxImage.h>
#include <utility/xxMaterial.h>
#include <utility/xxMesh.h>
#include <utility/xxNode.h>
#include "ImportFBX.h"

#include "ufbx/ufbx.h"

//==============================================================================
static std::string str(const ufbx_string& string)
{
    return std::string(string.data, string.length);
}
//------------------------------------------------------------------------------
static xxVector2 vec2(const ufbx_vec2& values)
{
    return { (float)values.x, (float)values.y };
}
//------------------------------------------------------------------------------
static xxVector3 vec3(const ufbx_vec3& values)
{
    return { (float)values.x, (float)values.y, (float)values.z };
}
//------------------------------------------------------------------------------
static xxVector4 vec4(const ufbx_vec4& values)
{
    return { (float)values.x, (float)values.y, (float)values.z, (float)values.w };
}
//------------------------------------------------------------------------------
static xxMatrix4x4 mat4(const ufbx_matrix& values)
{
    xxMatrix4x4 output = xxMatrix4x4::IDENTITY;
    output[0].xyz = vec3(values.cols[0]);
    output[1].xyz = vec3(values.cols[1]);
    output[2].xyz = vec3(values.cols[2]);
    output[3].xyz = vec3(values.cols[3]);
    return output;
}
//------------------------------------------------------------------------------
static xxMatrix4x4 mat4(const ufbx_transform& values)
{
    return mat4(ufbx_transform_to_matrix(&values));
}
//------------------------------------------------------------------------------
static xxImagePtr createImage(ufbx_texture* texture)
{
    if (texture == nullptr || texture->has_file == false)
        return xxImagePtr();
    xxImagePtr image = Import::CreateImage(texture->filename.data);
    if (image)
    {
        image->ClampU = (texture->wrap_u == UFBX_WRAP_CLAMP);
        image->ClampV = (texture->wrap_v == UFBX_WRAP_CLAMP);
        image->ClampW = (texture->wrap_u == UFBX_WRAP_CLAMP) && (texture->wrap_v == UFBX_WRAP_CLAMP);
    }
    return image;
}
//------------------------------------------------------------------------------
static xxMaterialPtr createMaterial(ufbx_material* material)
{
    if (material == nullptr)
        return xxMaterialPtr();
    xxMaterialPtr output = xxMaterial::Create();
    output->Name = str(material->shading_model_name);
    output->AmbientColor = vec3(material->fbx.ambient_color.value_vec3);
    output->DiffuseColor = vec3(material->fbx.diffuse_color.value_vec3);
    output->EmissiveColor = vec3(material->fbx.emission_color.value_vec3);
    output->SpecularColor = vec3(material->fbx.specular_color.value_vec3);
    output->SpecularHighlight = material->fbx.specular_exponent.value_real;
    output->Opacity = material->fbx.transparency_factor.value_real;
    switch (material->shader_type)
    {
    default:
    case UFBX_SHADER_FBX_LAMBERT:
        output->Lighting = true;
        break;
    case UFBX_SHADER_FBX_PHONG:
        output->Lighting = true;
        output->Specular = true;
        break;
    }
    output->DepthTest = "LessEqual";
    output->DepthWrite = true;
    output->Cull = true;
    output->Scissor = false;
    for (size_t i = 0; i < material->textures.count; ++i)
    {
        output->SetImage(i, createImage(material->textures.data[i].texture));
    }
    return output;
};
//------------------------------------------------------------------------------
static xxMeshPtr createMesh(ufbx_mesh* mesh)
{
    if (mesh == nullptr)
        return xxMeshPtr();
    int normalCount = 0;
    int colorCount = 0;
    int textureCount = 0;
    if (mesh->vertex_normal.exists)
    {
        normalCount = 1;
        if (mesh->vertex_tangent.exists)
        {
            normalCount = 2;
            if (mesh->vertex_bitangent.exists)
            {
                normalCount = 3;
            }
        }
    }
    colorCount = std::min((int)mesh->color_sets.count, 8);
    textureCount = std::min((int)mesh->uv_sets.count, 8);
    xxMeshPtr output = xxMesh::Create(normalCount, colorCount, textureCount);
    output->Name = str(mesh->name);
    output->SetVertexCount((int)mesh->num_indices);

    xxStrideIterator<xxVector3> vertices = output->GetVertex();
    xxStrideIterator<xxVector3> normals = output->GetNormal(0);
    xxStrideIterator<xxVector3> tangents = output->GetNormal(1);
    xxStrideIterator<xxVector3> bitangents = output->GetNormal(2);
    xxStrideIterator<uint32_t> colors[8] =
    {
        output->GetColor(0), output->GetColor(1), output->GetColor(2), output->GetColor(3),
        output->GetColor(4), output->GetColor(5), output->GetColor(6), output->GetColor(7),
    };
    xxStrideIterator<xxVector2> textures[8] =
    {
        output->GetTexture(0), output->GetTexture(1), output->GetTexture(2), output->GetTexture(3),
        output->GetTexture(4), output->GetTexture(5), output->GetTexture(6), output->GetTexture(7),
    };

    for (size_t i = 0; i < mesh->num_indices; ++i)
    {
        (*vertices++) = vec3(ufbx_get_vertex_vec3(&mesh->vertex_position, i));
        if (normalCount >= 1)
            (*normals++) = vec3(ufbx_get_vertex_vec3(&mesh->vertex_normal, i));
        if (normalCount >= 2)
            (*tangents++) = vec3(ufbx_get_vertex_vec3(&mesh->vertex_tangent, i));
        if (normalCount >= 3)
            (*bitangents++) = vec3(ufbx_get_vertex_vec3(&mesh->vertex_bitangent, i));
        for (size_t j = 0; j < colorCount; ++j)
            (*colors[j]++) = vec4(ufbx_get_vertex_vec4(&mesh->color_sets[j].vertex_color, i)).ToInteger();
        for (size_t j = 0; j < textureCount; ++j)
        {
            xxVector2 uv = vec2(ufbx_get_vertex_vec2(&mesh->uv_sets[j].vertex_uv, i));

            if (Import::EnableTextureFlipV)
            {
                uv.y = 1.0f - uv.y;
            }
            (*textures[j]++) = uv;
        }
    }

    return output;
}
//------------------------------------------------------------------------------
static xxNodePtr createNode(ufbx_node* node)
{
    if (node == nullptr)
        return xxNodePtr();
    xxNodePtr output = xxNode::Create();
    for (size_t i = 0; i < node->children.count; ++i)
    {
        xxNodePtr child = createNode(node->children.data[i]);
        if (child)
        {
            output->AttachChild(child);
        }
    }
    output->Name = str(node->name);
    output->LocalMatrix = mat4(node->local_transform);
    if (node->mesh)
    {
        xxNodePtr geometryNode = output;
        if (node->has_geometry_transform)
        {
            if (node->children.count == 0)
            {
                ufbx_matrix a = ufbx_transform_to_matrix(&node->local_transform);
                ufbx_matrix b = ufbx_transform_to_matrix(&node->geometry_transform);
                output->LocalMatrix = mat4(ufbx_matrix_mul(&a, &b));
            }
            else
            {
                geometryNode = xxNode::Create();
                geometryNode->Name = str(node->mesh->name);
                geometryNode->LocalMatrix = mat4(node->geometry_transform);
                output->AttachChild(geometryNode);
            }
        }
        if (node->mesh->materials.count)
        {
            ufbx_material* material = node->mesh->materials.data[0];
            geometryNode->Material = createMaterial(material);
        }
        geometryNode->Mesh = createMesh(node->mesh);
        if (Import::EnableOptimizeMesh)
        {
            geometryNode->Mesh = Import::OptimizeMesh(geometryNode->Mesh);
        }
    }
    return output;
}
//------------------------------------------------------------------------------
xxNodePtr ImportFBX::Create(char const* fbx)
{
    ufbx_load_opts opts = {};
    ufbx_error error;
    ufbx_scene* scene = ufbx_load_file(fbx, &opts, &error);
    if (scene == nullptr)
    {
        xxLog("ImportFBX", "Failed to load: %s", error.description.data);
        return nullptr;
    }

    xxNodePtr root = createNode(scene->root_node);
    if (root)
    {
        if (root->Name.empty())
        {
            root->Name = xxFile::GetName(fbx);
        }

        if (EnableAxisUpYToZ)
        {
            static const xxMatrix4 YtoZ =
            {
                1,  0, 0, 0,
                0,  0, 1, 0,
                0, -1, 0, 0,
                0,  0, 0, 1,
            };
            root->LocalMatrix = root->LocalMatrix * YtoZ;
        }
    }

    ufbx_free_scene(scene);

    return root;
}
//==============================================================================
