//==============================================================================
// Minamoto : Material Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxCamera.h>
#include <xxGraphicPlus/xxMesh.h>
#include <xxGraphicPlus/xxNode.h>
#include <xxGraphicPlus/xxTexture.h>
#include "Material.h"

//==============================================================================
//  Material
//==============================================================================
void Material::Invalidate()
{
    xxDestroyShader(m_device, m_meshShader);
    m_meshShader = 0;
    return xxMaterial::Invalidate();
}
//------------------------------------------------------------------------------
void Material::Draw(xxDrawData const& data) const
{
    auto* constantData = data.constantData;

    xxSetPipeline(data.commandEncoder, constantData->pipeline);
    if (constantData->meshConstant)
    {
        xxSetMeshConstantBuffer(data.commandEncoder, constantData->meshConstant, constantData->meshConstantSize);
    }
    if (constantData->vertexConstant)
    {
        xxSetVertexConstantBuffer(data.commandEncoder, constantData->vertexConstant, constantData->vertexConstantSize);
    }
    if (constantData->fragmentConstant)
    {
        xxSetFragmentConstantBuffer(data.commandEncoder, constantData->fragmentConstant, constantData->fragmentConstantSize);
    }

    int textureCount = 0;
    uint64_t textures[16] = {};
    uint64_t samplers[16] = {};

    unsigned int slot = m_fragmentTextureSlot;
    for (int i = 0; i < 16; ++i)
    {
        slot &= ~(1 << i);
        if (i >= Textures.size())
            continue;
        xxTexturePtr const& texture = Textures[i];
        texture->Update(data.device);
        textures[i] = texture->Texture;
        samplers[i] = texture->Sampler;
        textureCount = i + 1;
        if (slot == 0)
            break;
    }

    xxSetFragmentTextures(data.commandEncoder, textureCount, textures);
    xxSetFragmentSamplers(data.commandEncoder, textureCount, samplers);
}
//------------------------------------------------------------------------------
void Material::CreatePipeline(xxDrawData const& data)
{
    xxMesh* mesh = data.mesh;
    uint64_t vertexAttribute = mesh->GetVertexAttribute();
    if (vertexAttribute == 0)
        return;

    if (m_pipeline == 0)
    {
        if (m_blendState == 0)
        {
            if (Blending)
            {
                m_blendState = xxCreateBlendState(m_device, BlendSourceColor.c_str(),
                                                            BlendOperationColor.c_str(),
                                                            BlendDestinationColor.c_str(),
                                                            BlendSourceAlpha.c_str(),
                                                            BlendOperationAlpha.c_str(),
                                                            BlendDestinationAlpha.c_str());
            }
            else
            {
                m_blendState = xxCreateBlendState(m_device, "1", "+", "0", "1", "+", "0");
            }
        }
        if (m_depthStencilState == 0)
        {
            m_depthStencilState = xxCreateDepthStencilState(m_device, DepthTest.c_str(), DepthWrite);
        }
        if (m_rasterizerState == 0)
        {
            m_rasterizerState = xxCreateRasterizerState(m_device, Cull, Scissor);
        }
        if (m_meshShader == 0 && m_vertexShader == 0 && m_fragmentShader == 0)
        {
            if (m_meshShader == 0 && mesh->Count[xxMesh::STORAGE0] && mesh->Count[xxMesh::STORAGE1] && mesh->Count[xxMesh::STORAGE2])
            {
                m_meshShader = xxCreateMeshShader(m_device, GetShader(data, 'mesh').c_str());
            }
            if (m_meshShader == 0 && m_vertexShader == 0)
            {
                m_vertexShader = xxCreateVertexShader(m_device, GetShader(data, 'vert').c_str(), vertexAttribute);
            }
            if (m_fragmentShader == 0)
            {
                m_fragmentShader = xxCreateFragmentShader(m_device, GetShader(data, 'frag').c_str());
            }
        }
        if (m_renderPass == 0)
        {
            m_renderPass = xxCreateRenderPass(m_device, true, true, true, true, true, true);
        }
        m_pipeline = xxCreatePipeline(m_device, m_renderPass, m_blendState, m_depthStencilState, m_rasterizerState, vertexAttribute, m_meshShader, m_vertexShader, m_fragmentShader);
    }
}
//------------------------------------------------------------------------------
void Material::CreateConstant(xxDrawData const& data) const
{
    auto* constantData = data.constantData;

    if (constantData->meshConstant == 0 &&constantData->vertexConstant == 0 && constantData->fragmentConstant == 0)
    {
        constantData->device = data.device;
        constantData->pipeline = m_pipeline;
        if (m_meshShader && constantData->meshConstant == 0)
        {
            constantData->meshConstantSize = GetMeshConstantSize(data);
            if (constantData->meshConstantSize > 0)
            {
                constantData->meshConstant = xxCreateConstantBuffer(m_device, constantData->meshConstantSize);
            }
        }
        if (m_vertexShader && constantData->vertexConstant == 0)
        {
            constantData->vertexConstantSize = GetVertexConstantSize(data);
            if (constantData->vertexConstantSize > 0)
            {
                constantData->vertexConstant = xxCreateConstantBuffer(m_device, constantData->vertexConstantSize);
            }
        }
        if (m_fragmentShader && constantData->fragmentConstant == 0)
        {
            constantData->fragmentConstantSize = GetFragmentConstantSize(data);
            if (constantData->fragmentConstantSize > 0)
            {
                constantData->fragmentConstant = xxCreateConstantBuffer(m_device, constantData->fragmentConstantSize);
            }
        }
    }
}
//------------------------------------------------------------------------------
void Material::UpdateConstant(xxDrawData const& data) const
{
    int size;
    auto* constantData = data.constantData;

    size = constantData->meshConstantSize;
    if (size == 0)
        size = constantData->vertexConstantSize;
    if (size)
    {
        uint64_t constant = constantData->meshConstant;
        if (constant == 0)
            constant = constantData->vertexConstant;
        xxVector4* vector = reinterpret_cast<xxVector4*>(xxMapBuffer(m_device, constant));
        if (vector)
        {
            UpdateWorldViewProjectionConstant(data, size, &vector);
            UpdateCullingConstant(data, size, &vector);
            UpdateSkinningConstant(data, size, &vector);
            UpdateBlendingConstant(data, size, &vector);
            UpdateLightingConstant(data, size, &vector);
            xxUnmapBuffer(m_device, constant);
        }
    }

    size = constantData->fragmentConstantSize;
    if (size)
    {
        uint64_t constant = constantData->fragmentConstant;
        xxVector4* vector = reinterpret_cast<xxVector4*>(xxMapBuffer(m_device, constant));
        if (vector)
        {
            UpdateAlphaTestingConstant(data, size, &vector);
            UpdateLightingConstant(data, size, &vector);
            xxUnmapBuffer(m_device, constant);
        }
    }
}
//------------------------------------------------------------------------------
std::string Material::GetShader(xxDrawData const& data, int type) const
{
    xxMesh* mesh = data.mesh;

    auto define = [](char const* name, size_t value)
    {
        return std::string("#define") + ' ' + name + ' ' + std::to_string(value) + '\n';
    };

    size_t constantSize = 0;
    uint16_t meshTextureSlot = 0;
    uint16_t vertexTextureSlot = 0;
    uint16_t fragmentTextureSlot = 1;
    std::string shader;

    switch (type)
    {
    case 'mesh':
        constantSize = GetMeshConstantSize(data) / sizeof(xxVector4);
        shader += define("SHADER_BACKFACE_CULLING", BackfaceCulling ? 1 : 0);
        shader += define("SHADER_FRUSTUM_CULLING", FrustumCulling ? 1 : 0);
        shader += define("SHADER_SKINNING", mesh->Skinning ? 1 : 0);
        shader += define("SHADER_OPACITY", Blending ? 1 : 0);
        break;
    case 'vert':
        constantSize = GetVertexConstantSize(data) / sizeof(xxVector4);
        shader += define("SHADER_SKINNING", mesh->Skinning ? 1 : 0);
        shader += define("SHADER_OPACITY", Blending ? 1 : 0);
        break;
    case 'frag':
        constantSize = GetFragmentConstantSize(data) / sizeof(xxVector4);
        if (GetTexture(BUMP))
        {
            fragmentTextureSlot |= 0b10;
        }
        shader += define("SHADER_ALPHATEST", AlphaTest ? 1 : 0);
        break;
    }

    const_cast<uint16_t&>(m_meshTextureSlot) = meshTextureSlot;
    const_cast<uint16_t&>(m_vertexTextureSlot) = vertexTextureSlot;
    const_cast<uint16_t&>(m_fragmentTextureSlot) = fragmentTextureSlot;

    shader += define("SHADER_NORMAL", mesh->NormalCount);
    shader += define("SHADER_COLOR", mesh->ColorCount);
    shader += define("SHADER_TEXTURE", mesh->TextureCount);
    shader += define("SHADER_UNIFORM", constantSize);
    shader += define("SHADER_LIGHTING", mesh->NormalCount && Lighting ? 1 : 0);
    shader += define("SHADER_SPECULAR", mesh->NormalCount && Lighting && Specular ? 1 : 0);
    shader += define("TEXTURE_BASE", mesh->TextureCount && GetTexture(BASE) ? 1 : 0);
    shader += define("TEXTURE_BUMP", mesh->TextureCount && GetTexture(BUMP) ? 1 : 0);
    shader += ShaderOption;
    shader += Shader.empty() ? DefaultShader : Shader;

    return shader;
}
//------------------------------------------------------------------------------
int Material::GetMeshConstantSize(xxDrawData const& data) const
{
    int size = 0;
    UpdateWorldViewProjectionConstant(data, size);
    UpdateCullingConstant(data, size);
    UpdateSkinningConstant(data, size);
    UpdateBlendingConstant(data, size);
    UpdateLightingConstant(data, size);
    return size;
}
//------------------------------------------------------------------------------
int Material::GetVertexConstantSize(xxDrawData const& data) const
{
    int size = 0;
    UpdateWorldViewProjectionConstant(data, size);
    UpdateSkinningConstant(data, size);
    UpdateBlendingConstant(data, size);
    UpdateLightingConstant(data, size);
    return size;
}
//------------------------------------------------------------------------------
int Material::GetFragmentConstantSize(xxDrawData const& data) const
{
    int size = 0;
    UpdateAlphaTestingConstant(data, size);
    UpdateLightingConstant(data, size);
    return size;
}
//------------------------------------------------------------------------------
void Material::UpdateAlphaTestingConstant(xxDrawData const& data, int& size, xxVector4** pointer) const
{
    if (AlphaTest == false)
        return;
    if (pointer == nullptr)
    {
        size += 1 * sizeof(xxVector4);
    }
    if (size >= 1 * sizeof(xxVector4) && pointer)
    {
        xxVector4* vector = (*pointer);
        size -= 1 * sizeof(xxVector4);
        (*pointer) += 1;

        vector[0].x = AlphaTestReference;
    }
}
//------------------------------------------------------------------------------
void Material::UpdateBlendingConstant(xxDrawData const& data, int& size, xxVector4** pointer) const
{
    if (Blending == false)
        return;
    if (pointer == nullptr)
    {
        size += 1 * sizeof(xxVector4);
    }
    if (size >= 1 * sizeof(xxVector4) && pointer)
    {
        xxVector4* vector = (*pointer);
        size -= 1 * sizeof(xxVector4);
        (*pointer) += 1;

        vector[0].x = Opacity;
    }
}
//------------------------------------------------------------------------------
void Material::UpdateCullingConstant(xxDrawData const& data, int& size, xxVector4** pointer) const
{
    if (m_meshShader == 0)
        return;
    if (BackfaceCulling == false && FrustumCulling == false)
        return;
    if (pointer == nullptr)
    {
        size += 6 * sizeof(xxMatrix4x2);
    }
    if (size >= 6 * sizeof(xxMatrix4x2) && pointer)
    {
        xxMatrix4x2* frustum = reinterpret_cast<xxMatrix4x2*>(*pointer);
        size -= 6 * sizeof(xxMatrix4x2);
        (*pointer) += 6 * 2;

        if (data.frustum)
        {
            for (int i = 0; i < 6; ++i)
            {
                frustum[i] = data.frustum[i];
            }
        }
        else
        {
            for (int i = 0; i < 6; ++i)
            {
                frustum[i] = {};
            }
        }
    }
}
//------------------------------------------------------------------------------
void Material::UpdateLightingConstant(xxDrawData const& data, int& size, xxVector4** pointer) const
{
    if (Lighting == false)
        return;
    if (pointer == nullptr)
    {
        size += 7 * sizeof(xxVector4);
    }
    if (size >= 7 * sizeof(xxVector4) && pointer)
    {
        xxVector4* vector = (*pointer);
        size -= 7 * sizeof(xxVector4);
        (*pointer) += 7;

        xxCamera* camera = data.camera;
        if (camera)
        {
            vector[0].xyz = camera->Location;
            vector[1].xyz = camera->LightDirection;
            vector[2].xyz = camera->LightColor;
        }
        else
        {
            vector[0].xyz = xxVector3::ZERO;
            vector[1].xyz = xxVector3::Y;
            vector[2].xyz = xxVector3::WHITE;
        }
        vector[3].xyz = AmbientColor;
        vector[4].xyz = DiffuseColor;
        vector[5].xyz = EmissiveColor;
        vector[6].xyz = SpecularColor;
        vector[6].w = SpecularHighlight;
    }
}
//------------------------------------------------------------------------------
void Material::UpdateSkinningConstant(xxDrawData const& data, int& size, xxVector4** pointer) const
{
    if (data.mesh->Skinning == false)
        return;
    if (pointer == nullptr)
    {
        size += 75 * sizeof(xxMatrix4x3);
    }
    if (size >= 75 * sizeof(xxMatrix4x3) && pointer)
    {
        xxMatrix4x3* boneMatrix = reinterpret_cast<xxMatrix4x3*>(*pointer);
        size -= 75 * sizeof(xxMatrix4x3);
        (*pointer) += 75 * 3;

        for (auto const& data : data.node->Bones)
        {
            (*boneMatrix++) = xxMatrix4x3::FromMatrix4(data.boneMatrix);
        }
    }
}
//------------------------------------------------------------------------------
void Material::UpdateWorldViewProjectionConstant(xxDrawData const& data, int& size, xxVector4** pointer) const
{
    if (pointer == nullptr)
    {
        size += 3 * sizeof(xxMatrix4x4);
    }
    if (size >= 3 * sizeof(xxMatrix4x4) && pointer)
    {
        xxMatrix4x4* wvp = reinterpret_cast<xxMatrix4x4*>(*pointer);
        size -= 3 * sizeof(xxMatrix4x4);
        (*pointer) += 3 * 4;

        xxCamera* camera = data.camera;
        wvp[0] = data.node->WorldMatrix;
        if (camera)
        {
            wvp[1] = camera->ViewMatrix;
            wvp[2] = camera->ProjectionMatrix;
        }
        else
        {
            wvp[1] = xxMatrix4::IDENTITY;
            wvp[2] = xxMatrix4::IDENTITY;
        }
    }
}
//------------------------------------------------------------------------------
static xxMaterialPtr (*backupBinaryCreate)();
//------------------------------------------------------------------------------
void Material::Initialize()
{
    if (backupBinaryCreate)
        return;
    backupBinaryCreate = xxMaterial::BinaryCreate;

    xxMaterial::BinaryCreate = []() -> xxMaterialPtr
    {
        xxMaterialPtr material = xxMaterialPtr(new Material(), [](Material* material) { delete material; });
        if (material == nullptr)
            return nullptr;

        return material;
    };
}
//------------------------------------------------------------------------------
void Material::Shutdown()
{
    if (backupBinaryCreate == nullptr)
        return;
    xxMaterial::BinaryCreate = backupBinaryCreate;
}
//==============================================================================
//  Default Shader
//==============================================================================
char const Material::DefaultShader[] =
//------------------------------------------------------------------------------
//  Default
//------------------------------------------------------------------------------
R"(
#ifndef SHADER_GLSL
#define SHADER_GLSL 0
#endif
#ifndef SHADER_HLSL
#define SHADER_HLSL 0
#endif
#ifndef SHADER_MSL
#define SHADER_MSL 0
#endif
#ifndef SHADER_MESH
#define SHADER_MESH 0
#endif
#ifndef SHADER_VERTEX
#define SHADER_VERTEX 0
#endif
#ifndef SHADER_FRAGMENT
#define SHADER_FRAGMENT 0
#endif
#ifndef SHADER_BACKFACE_CULLING
#define SHADER_BACKFACE_CULLING 0
#endif
#ifndef SHADER_FRUSTUM_CULLING
#define SHADER_FRUSTUM_CULLING 0
#endif
#ifndef SHADER_SKINNING
#define SHADER_SKINNING 0
#endif
#ifndef SHADER_NORMAL
#define SHADER_NORMAL 0
#endif
#ifndef SHADER_COLOR
#define SHADER_COLOR 1
#endif
#ifndef SHADER_TEXTURE
#define SHADER_TEXTURE 1
#endif
#ifndef SHADER_UNIFORM
#define SHADER_UNIFORM 12
#endif
#ifndef SHADER_ALPHATEST
#define SHADER_ALPHATEST 0
#endif
#ifndef SHADER_OPACITY
#define SHADER_OPACITY 0
#endif
#ifndef SHADER_LIGHTING
#define SHADER_LIGHTING 0
#endif
#ifndef SHADER_SPECULAR
#define SHADER_SPECULAR 0
#endif
#ifndef TEXTURE_BASE
#define TEXTURE_BASE 0
#endif
#ifndef TEXTURE_BUMP
#define TEXTURE_BUMP 0
#endif)"
//------------------------------------------------------------------------------
//  Compatible type
//------------------------------------------------------------------------------
R"(
#if SHADER_GLSL
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float2x2 mat2
#define float3x3 mat3
#define float4x4 mat4
#define int2 ivec2
#define int3 ivec3
#define int4 ivec4
#endif)"
//------------------------------------------------------------------------------
//  mul
//------------------------------------------------------------------------------
R"(
#if SHADER_GLSL || SHADER_MSL
#define mul(a, b) (b * a)
#endif)"
//------------------------------------------------------------------------------
//  Attribute
//------------------------------------------------------------------------------
R"(
#if SHADER_GLSL
#if SHADER_VERTEX
attribute vec3 attrPosition;
#if SHADER_SKINNING
attribute vec3 attrBoneWeight;
attribute vec4 attrBoneIndices;
#endif
#if SHADER_NORMAL > 0
attribute vec3 attrNormal;
#if SHADER_NORMAL > 1
attribute vec3 attrTangent;
#if SHADER_NORMAL > 2
attribute vec3 attrBinormal;
#endif
#endif
#endif
#if SHADER_COLOR
attribute vec4 attrColor;
#endif
#if SHADER_TEXTURE
attribute vec2 attrUV0;
#endif
#endif
#elif SHADER_HLSL
#if SHADER_VERTEX
struct Attribute
{
    float3 Position     : POSITION;
#if SHADER_SKINNING
    float3 BoneWeight   : BLENDWEIGHT;
    uint4 BoneIndices   : BLENDINDICES;
#endif
#if SHADER_NORMAL > 0
    float3 Normal       : NORMAL;
#if SHADER_NORMAL > 1
    float3 Tangent      : TANGENT;
#if SHADER_NORMAL > 2
    float3 Binormal     : BINORMAL;
#endif
#endif
#endif
#if SHADER_COLOR
    float4 Color        : COLOR;
#endif
#if SHADER_TEXTURE
    float2 UV0          : TEXCOORD;
#endif
};
#endif
#elif SHADER_MSL
#if SHADER_MESH
struct Vertex
{
    packed_float3 Position;
#if SHADER_SKINNING
    packed_float3 BoneWeight;
    packed_uint4 BoneIndices;
#endif
#if SHADER_NORMAL > 0
    packed_float3 Normal;
#if SHADER_NORMAL > 1
    packed_float3 Tangent;
#if SHADER_NORMAL > 2
    packed_float3 Binormal;
#endif
#endif
#endif
#if SHADER_COLOR
    packed_float4 Color;
#endif
#if SHADER_TEXTURE
    packed_float2 UV0;
#endif
};
#elif SHADER_VERTEX
struct Attribute
{
    float3 Position     [[attribute(__COUNTER__)]];
#if SHADER_SKINNING
    float3 BoneWeight   [[attribute(__COUNTER__)]];
    uint4 BoneIndices   [[attribute(__COUNTER__)]];
#endif
#if SHADER_NORMAL > 0
    float3 Normal       [[attribute(__COUNTER__)]];
#if SHADER_NORMAL > 1
    float3 Tangent      [[attribute(__COUNTER__)]];
#if SHADER_NORMAL > 2
    float3 Binormal     [[attribute(__COUNTER__)]];
#endif
#endif
#endif
#if SHADER_COLOR
    float4 Color        [[attribute(__COUNTER__)]];
#endif
#if SHADER_TEXTURE
    float2 UV0          [[attribute(__COUNTER__)]];
#endif
};
#endif
#endif)"
//------------------------------------------------------------------------------
//  Varying
//------------------------------------------------------------------------------
R"(
#if SHADER_GLSL
#if SHADER_COLOR || SHADER_LIGHTING
varying vec4 varyColor;
#if SHADER_SPECULAR || TEXTURE_BUMP
varying vec3 varyWorldPosition;
#if SHADER_NORMAL > 0
varying vec3 varyWorldNormal;
#if SHADER_NORMAL > 1
varying vec3 varyWorldTangent;
#if SHADER_NORMAL > 2
varying vec3 varyWorldBinormal;
#endif
#endif
#endif
#endif
#endif
#if SHADER_TEXTURE
varying vec2 varyUV0;
#endif
#elif SHADER_HLSL
struct Varying
{
    float4 Position         : SV_POSITION;
#if SHADER_COLOR || SHADER_LIGHTING
    float4 Color            : COLOR;
#if SHADER_SPECULAR || TEXTURE_BUMP
    float3 WorldPosition    : COLOR1;
#if SHADER_NORMAL > 0
    float3 WorldNormal      : COLOR2;
#if SHADER_NORMAL > 1
    float3 WorldTangent     : COLOR3;
#if SHADER_NORMAL > 2
    float3 WorldBinormal    : COLOR4;
#endif
#endif
#endif
#endif
#endif
#if SHADER_TEXTURE
    float2 UV0              : TEXCOORD0;
#endif
};
#elif SHADER_MSL
struct Varying
{
    float4 Position [[position]];
#if SHADER_COLOR || SHADER_LIGHTING
    float4 Color;
#if SHADER_SPECULAR || TEXTURE_BUMP
    float3 WorldPosition;
#if SHADER_NORMAL > 0
    float3 WorldNormal;
#if SHADER_NORMAL > 1
    float3 WorldTangent;
#if SHADER_NORMAL > 2
    float3 WorldBinormal;
#endif
#endif
#endif
#endif
#endif
#if SHADER_TEXTURE
    float2 UV0;
#endif
};
#endif)"
//------------------------------------------------------------------------------
//  Uniform
//------------------------------------------------------------------------------
R"(
#if SHADER_MESH
struct Meshlet
{
    uint VertexOffset;
    uint TriangleOffset;
    uint VertexCount;
    uint TriangleCount;
    float4 CenterRadius;
    float4 ConeApex;
    float4 ConeAxisCutoff;
};
#endif
#if SHADER_GLSL || SHADER_HLSL
#if SHADER_UNIFORM
uniform float4 uniBuffer[SHADER_UNIFORM];
#endif
#if SHADER_HLSL >= 10
sampler samDiffuse;
sampler samBump;
Texture2D<float4> texDiffuse;
Texture2D<float4> texBump;
#else
uniform sampler2D samDiffuse;
uniform sampler2D samBump;
#endif
#elif SHADER_MSL
struct Uniform
{
#if SHADER_MSL >= 2
#if SHADER_MESH
    device Vertex* Vertices     [[id(0)]];
    device Meshlet* Meshlets    [[id(1)]];
    device uint* VertexIndices  [[id(2)]];
    device uint* TriangeIndices [[id(3)]];
    device float4* Buffer       [[id(30)]];
#elif SHADER_VERTEX
    device float4* Buffer       [[id(0)]];
#else
    device float4* Buffer       [[id(1)]];
    texture2d<float> Diffuse    [[id(4)]];
    texture2d<float> Bump       [[id(5)]];
    sampler DiffuseSampler      [[id(18)]];
    sampler BumpSampler         [[id(19)]];
#endif
#elif SHADER_UNIFORM
    float4 Buffer[SHADER_UNIFORM];
#endif
};
struct Sampler
{
    texture2d<float> Diffuse    [[texture(0)]];
    texture2d<float> Bump       [[texture(1)]];
    sampler DiffuseSampler      [[sampler(0)]];
    sampler BumpSampler         [[sampler(1)]];
};
#endif)"
//------------------------------------------------------------------------------
//  Mesh / Vertex Shader
//------------------------------------------------------------------------------
R"(
#if SHADER_MESH
#if SHADER_MSL
[[mesh]]
#if SHADER_MSL >= 2
void Main(constant Uniform& uni [[buffer(0)]],
#else
void Main(constant Uniform& uni [[buffer(8)]],
          device Vertex* Vertices [[buffer(0)]],
          device Meshlet* Meshlets [[buffer(1)]],
          device uint* VertexIndices [[buffer(2)]],
          device uint* TriangeIndices [[buffer(3)]],
#endif
          uint gtid [[thread_position_in_threadgroup]],
          uint gid [[threadgroup_position_in_grid]],
          mesh<Varying, void, 64, 128, topology::triangle> output)
#endif
#endif
#if SHADER_VERTEX
#if SHADER_GLSL
void main()
#elif SHADER_HLSL
Varying Main(Attribute attr)
#elif SHADER_MSL
vertex Varying Main(Attribute attr [[stage_in]], constant Uniform& uni [[buffer(0)]])
#endif
#endif
#if SHADER_MESH || SHADER_VERTEX
{
    int uniIndex = 0;
#if SHADER_MSL
    auto uniBuffer = uni.Buffer;
#endif

#if SHADER_MESH
#if SHADER_MSL
#if SHADER_MSL >= 2
    device Vertex* Vertices = uni.Vertices;
    device Meshlet* Meshlets = uni.Meshlets;
    device uint* VertexIndices = uni.VertexIndices;
    device uint* TriangeIndices = uni.TriangeIndices;
#endif
    device Meshlet& m = Meshlets[gid];
#if SHADER_BACKFACE_CULLING || SHADER_FRUSTUM_CULLING
    uint visible = 1;
    if (simd_is_first())
    {
#if SHADER_BACKFACE_CULLING
        if (visible)
        {
            float c = m.ConeAxisCutoff.w; 
            float d = dot(m.ConeAxisCutoff.xyz, normalize(m.ConeApex.xyz - uniBuffer[13].xyz));
            if (d >= c)
                visible = 0;
        }
#endif
#if SHADER_FRUSTUM_CULLING
        if (visible)
        {
            float r = -m.CenterRadius.w;
            float d0 = dot(uniBuffer[12].xyz, m.CenterRadius.xyz - uniBuffer[13].xyz);
            float d1 = dot(uniBuffer[14].xyz, m.CenterRadius.xyz - uniBuffer[15].xyz);
            float d2 = dot(uniBuffer[16].xyz, m.CenterRadius.xyz - uniBuffer[17].xyz);
            float d3 = dot(uniBuffer[18].xyz, m.CenterRadius.xyz - uniBuffer[19].xyz);
            float d4 = dot(uniBuffer[20].xyz, m.CenterRadius.xyz - uniBuffer[21].xyz);
            float d5 = dot(uniBuffer[22].xyz, m.CenterRadius.xyz - uniBuffer[23].xyz);
            if (d0 < r || d1 < r || d2 < r || d3 < r || d4 < r || d5 < r)
                visible = 0;
        }
#endif
    }
    visible = simd_broadcast_first(visible);
    if (visible == 0)
        return;
    uniIndex += 12;
#endif
    if (gtid == 0)
    {
        output.set_primitive_count(m.TriangleCount);
    }

    if (gtid < m.TriangleCount)
    {
        uint index = 3 * gtid;
        uint packed = TriangeIndices[m.TriangleOffset + gtid];
        output.set_index(index + 0, (packed >>  0) & 0xFF);
        output.set_index(index + 1, (packed >>  8) & 0xFF);
        output.set_index(index + 2, (packed >> 16) & 0xFF);
    }

    if (gtid >= m.VertexCount)
        return;
    uint vertexIndex = VertexIndices[m.VertexOffset + gtid];
    device Vertex& attr = Vertices[vertexIndex];
#endif
#endif
#if SHADER_HLSL || SHADER_MSL
    float3 attrPosition = attr.Position;
#if SHADER_SKINNING
    float3 attrBoneWeight = attr.BoneWeight;
    uint4 attrBoneIndices = attr.BoneIndices;
#endif
#if SHADER_NORMAL > 0
    float3 attrNormal = attr.Normal;
#if SHADER_NORMAL > 1
    float3 attrTangent = attr.Tangent;
#if SHADER_NORMAL > 2
    float3 attrBinormal = attr.Binormal;
#endif
#endif
#endif
#if SHADER_COLOR
    float4 attrColor = attr.Color;
#endif
#if SHADER_TEXTURE
    float2 attrUV0 = attr.UV0;
#endif
#endif

    float4x4 world = float4x4(uniBuffer[0], uniBuffer[1], uniBuffer[2], uniBuffer[3]);
    float4x4 view = float4x4(uniBuffer[4], uniBuffer[5], uniBuffer[6], uniBuffer[7]);
    float4x4 projection = float4x4(uniBuffer[8], uniBuffer[9], uniBuffer[10], uniBuffer[11]);
    uniIndex += 12;

#if SHADER_SKINNING
    float4 zero4 = float4(0.0, 0.0, 0.0, 0.0);
    float4 boneWeight = float4(attrBoneWeight, 1.0 - attrBoneWeight.x - attrBoneWeight.y - attrBoneWeight.z);
    int4 boneIndices = int4(attrBoneIndices);
    world  = float4x4(uniBuffer[boneIndices.x * 3 + 12], uniBuffer[boneIndices.x * 3 + 13], uniBuffer[boneIndices.x * 3 + 14], zero4) * boneWeight.x;
    world += float4x4(uniBuffer[boneIndices.y * 3 + 12], uniBuffer[boneIndices.y * 3 + 13], uniBuffer[boneIndices.y * 3 + 14], zero4) * boneWeight.y;
    world += float4x4(uniBuffer[boneIndices.z * 3 + 12], uniBuffer[boneIndices.z * 3 + 13], uniBuffer[boneIndices.z * 3 + 14], zero4) * boneWeight.z;
    world += float4x4(uniBuffer[boneIndices.w * 3 + 12], uniBuffer[boneIndices.w * 3 + 13], uniBuffer[boneIndices.w * 3 + 14], zero4) * boneWeight.w;
    world = transpose(world);
    world[3][3] = 1.0;
    uniIndex += 75 * 3;
#endif
    float4 worldPosition = mul(float4(attrPosition, 1.0), world);
    float4 screenPosition = mul(mul(worldPosition, view), projection);

    float4 color = float4(1.0, 1.0, 1.0, 1.0);
#if SHADER_COLOR
    color = attrColor;
#endif
#if SHADER_OPACITY
    color.a = uniBuffer[uniIndex++].x;
#endif

#if SHADER_LIGHTING
#if SHADER_NORMAL > 0
    float3 worldNormal = normalize(mul(float4(attrNormal, 1.0), world).xyz);
#if SHADER_NORMAL > 1
    float3 worldTangent = normalize(mul(float4(attrTangent, 1.0), world).xyz);
#if SHADER_NORMAL > 2
    float3 worldBinormal = normalize(mul(float4(attrBinormal, 1.0), world).xyz);
#endif
#endif
#endif
#if TEXTURE_BUMP == 0
    float3 cameraPosition = uniBuffer[uniIndex++].xyz;
    float3 lightDirection = uniBuffer[uniIndex++].xyz;
    float3 lightColor = uniBuffer[uniIndex++].xyz;
    float3 ambientColor = uniBuffer[uniIndex++].xyz;
    float3 diffuseColor = uniBuffer[uniIndex++].xyz;
    float3 emissiveColor = uniBuffer[uniIndex++].xyz;
    float3 N = worldNormal;
    float3 L = lightDirection;
    float lambert = dot(N, L);
    color.rgb = color.rgb * (ambientColor + diffuseColor * lightColor * lambert) + emissiveColor;
#endif
#endif

#if SHADER_GLSL
    gl_Position = screenPosition;
#if SHADER_COLOR || SHADER_LIGHTING
    varyColor = color;
#if SHADER_SPECULAR || TEXTURE_BUMP
    varyWorldPosition = worldPosition.xyz;
#if SHADER_NORMAL > 0
    varyWorldNormal = worldNormal;
#if SHADER_NORMAL > 1
    varyWorldTangent = worldTangent;
#if SHADER_NORMAL > 2
    varyWorldBinormal = worldBinormal;
#endif
#endif
#endif
#endif
#endif
#if SHADER_TEXTURE
    varyUV0 = attrUV0;
#endif
#elif SHADER_HLSL || SHADER_MSL
    Varying vary;
    vary.Position = screenPosition;
#if SHADER_COLOR || SHADER_LIGHTING
    vary.Color = color;
#if SHADER_SPECULAR || TEXTURE_BUMP
    vary.WorldPosition = worldPosition.xyz;
#if SHADER_NORMAL > 0
    vary.WorldNormal = worldNormal;
#if SHADER_NORMAL > 1
    vary.WorldTangent = worldTangent;
#if SHADER_NORMAL > 2
    vary.WorldBinormal = worldBinormal;
#endif
#endif
#endif
#endif
#endif
#if SHADER_TEXTURE
    vary.UV0 = attrUV0;
#endif
#if SHADER_MESH
    output.set_vertex(gtid, vary);
#endif
#if SHADER_VERTEX
    return vary;
#endif
#endif
}
#endif)"
//------------------------------------------------------------------------------
//  Fragment Shader
//------------------------------------------------------------------------------
R"(
#if SHADER_FRAGMENT
#if SHADER_GLSL
void main()
#elif SHADER_HLSL
float4 Main(Varying vary) : COLOR0
#elif SHADER_MSL
fragment float4 Main(Varying vary [[stage_in]], constant Uniform& uni [[buffer(0)]], Sampler sam)
#endif
{
#if SHADER_HLSL || SHADER_MSL
#if SHADER_COLOR || SHADER_LIGHTING
    float4 varyColor = vary.Color;
#if SHADER_SPECULAR || TEXTURE_BUMP
    float3 varyWorldPosition = vary.WorldPosition;
#if SHADER_NORMAL > 0
    float3 varyWorldNormal = vary.WorldNormal;
#if SHADER_NORMAL > 1
    float3 varyWorldTangent = vary.WorldTangent;
#if SHADER_NORMAL > 2
    float3 varyWorldBinormal = vary.WorldBinormal;
#endif
#endif
#endif
#endif
#endif
#endif

    float4 color = float4(1.0, 1.0, 1.0, 1.0);
    float4 bump = float4(0.0, 0.0, 1.0, 0.0);
#if SHADER_COLOR || SHADER_LIGHTING
    color = varyColor;
#endif

#if TEXTURE_BASE
#if SHADER_GLSL
    color = color * texture2D(samDiffuse, varyUV0);
#elif SHADER_HLSL
#if SHADER_HLSL >= 10
    color = color * texDiffuse.Sample(samDiffuse, vary.UV0);
#else
    color = color * tex2D(samDiffuse, vary.UV0);
#endif
#elif SHADER_MSL
#if SHADER_MSL >= 2
    color = color * uni.Diffuse.sample(uni.DiffuseSampler, vary.UV0);
#else
    color = color * sam.Diffuse.sample(sam.DiffuseSampler, vary.UV0);
#endif
#endif
#endif

#if TEXTURE_BUMP
#if SHADER_GLSL
    bump = texture2D(samBump, varyUV0);
#elif SHADER_HLSL
#if SHADER_HLSL >= 10
    bump = texBump.Sample(samBump, vary.UV0);
#else
    bump = tex2D(samBump, vary.UV0);
#endif
#elif SHADER_MSL
#if SHADER_MSL >= 2
    bump = uni.Bump.sample(uni.BumpSampler, vary.UV0);
#else
    bump = sam.Bump.sample(sam.BumpSampler, vary.UV0);
#endif
#endif
    bump = bump * 2.0 - 1.0;
#endif

#if SHADER_UNIFORM
#if SHADER_MSL
    auto uniBuffer = uni.Buffer;
#endif
    int uniIndex = 0;
#if SHADER_ALPHATEST
    float alphaRef = uniBuffer[uniIndex++].x;
    if (color.a < alphaRef)
    {
#if SHADER_GLSL
        discard;
#elif SHADER_HLSL
        clip(-1);
#elif SHADER_MSL
        discard_fragment();
#endif
    }
#endif
#if SHADER_SPECULAR || TEXTURE_BUMP
    float3 cameraPosition = uniBuffer[uniIndex++].xyz;
    float3 lightDirection = uniBuffer[uniIndex++].xyz;
    float3 lightColor = uniBuffer[uniIndex++].xyz;
    float3 ambientColor = uniBuffer[uniIndex++].xyz;
    float3 diffuseColor = uniBuffer[uniIndex++].xyz;
    float3 emissiveColor = uniBuffer[uniIndex++].xyz;
    float3 specularColor = uniBuffer[uniIndex].xyz;
    float specularHighlight = uniBuffer[uniIndex++].w;
    float3 L = lightDirection;
    float3 N = varyWorldNormal;
#if TEXTURE_BUMP
    N.z = dot(varyWorldNormal, bump.xyz);
    N.x = dot(varyWorldTangent, bump.xyz);
    N.y = dot(varyWorldBinormal, bump.xyz);
    float lambert = dot(N, L);
    color.rgb = color.rgb * (ambientColor + diffuseColor * lightColor * lambert) + emissiveColor;
#endif
#if SHADER_SPECULAR
    float3 V = normalize(cameraPosition - varyWorldPosition);
    float3 H = normalize(V + L);
    float phong = pow(max(dot(N, H), 0.0001), specularHighlight);
    color.rgb = color.rgb + specularColor * phong;
#endif
#endif
#endif

#if SHADER_GLSL
    gl_FragColor = color;
#elif SHADER_HLSL || SHADER_MSL
    return color;
#endif
}
#endif)";
