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
struct MaterialSelector
{
    std::string& shader;
    int language;
    int type;
    int tab = 0;
    MaterialSelector(std::string& s, int t, int l) : shader(s), language(l), type(t) {}
    void Append(std::string_view string)
    {
        if (string.size())
        {
            tab -= (string[0] == '}' || string[0] == ')') ? 1 : 0;
            shader.append(4 * tab, ' ');
            shader += string;
            shader += '\n';
            tab += (string[0] == '{' || string[0] == '(') ? 1 : 0;
        }
    }
    void operator () (bool available, std::string_view string)
    {
        if (available == false)      return;
        Append(string);
    }
    void GM(bool available, std::string_view glsl, std::string_view msl)
    {
        std::string_view string;
        if (available == false)      return;
        else if (language == 'GLSL') { string = glsl; }
        else if (language == 'HLSL') { string = glsl; }
        else if (language == 'hlsl') { string = glsl; }
        else if (language == 'MSL1') { string = msl;  }
        else if (language == 'MSL2') { string = msl;  }
        Append(string);
    }
    void GH(bool available, std::string_view glsl, std::string_view hlsl)
    {
        std::string_view string;
        if (available == false)      return;
        else if (language == 'GLSL') { string = glsl; }
        else if (language == 'HLSL') { string = hlsl; }
        else if (language == 'hlsl') { string = hlsl; }
        else if (language == 'MSL1') { string = hlsl;  }
        else if (language == 'MSL2') { string = hlsl;  }
        Append(string);
    }
    void GHM(bool available, std::string_view glsl, std::string_view hlsl, std::string_view msl)
    {
        std::string_view string;
        if (available == false)      return;
        else if (language == 'GLSL') { string = glsl; }
        else if (language == 'HLSL') { string = hlsl; }
        else if (language == 'hlsl') { string = hlsl; }
        else if (language == 'MSL1') { string = msl;  }
        else if (language == 'MSL2') { string = msl;  }
        Append(string);
    }
    void GHHM(bool available, std::string_view glsl, std::string_view hlsl, std::string_view hlsl10, std::string_view msl)
    {
        std::string_view string;
        if (available == false)      return;
        else if (language == 'GLSL') { string = glsl;   }
        else if (language == 'HLSL') { string = hlsl10; }
        else if (language == 'hlsl') { string = hlsl;   }
        else if (language == 'MSL1') { string = msl;    }
        else if (language == 'MSL2') { string = msl;    }
        Append(string);
    }
    void GHMM(bool available, std::string_view glsl, std::string_view hlsl, std::string_view msl1, std::string_view msl2)
    {
        std::string_view string;
        if (available == false)      return;
        else if (language == 'GLSL') { string = glsl; }
        else if (language == 'HLSL') { string = hlsl; }
        else if (language == 'hlsl') { string = hlsl; }
        else if (language == 'MSL1') { string = msl1; }
        else if (language == 'MSL2') { string = msl2; }
        Append(string);
    }
    void HM(bool available, std::string_view hlsl, std::string_view msl)
    {
        std::string_view string;
        if (available == false)      return;
        else if (language == 'GLSL') { string = hlsl; }
        else if (language == 'HLSL') { string = hlsl; }
        else if (language == 'hlsl') { string = hlsl; }
        else if (language == 'MSL1') { string = msl;  }
        else if (language == 'MSL2') { string = msl;  }
        Append(string);
    }
    void HMM(bool available, std::string_view hlsl, std::string_view msl1, std::string_view msl2)
    {
        std::string_view string;
        if (available == false)      return;
        else if (language == 'GLSL') { string = hlsl; }
        else if (language == 'HLSL') { string = hlsl; }
        else if (language == 'hlsl') { string = hlsl; }
        else if (language == 'MSL1') { string = msl1; }
        else if (language == 'MSL2') { string = msl2; }
        Append(string);
    }
    void HHMM(bool available, std::string_view hlsl, std::string_view hlsl10, std::string_view msl1, std::string_view msl2)
    {
        std::string_view string;
        if (available == false)      return;
        else if (language == 'GLSL') { string = hlsl;   }
        else if (language == 'HLSL') { string = hlsl10; }
        else if (language == 'hlsl') { string = hlsl;   }
        else if (language == 'MSL1') { string = msl1;   }
        else if (language == 'MSL2') { string = msl2;   }
        Append(string);
    }
};
//------------------------------------------------------------------------------
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
            UpdateCullingConstant(data, size, &vector);
            UpdateWorldViewProjectionConstant(data, size, &vector);
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

    char const* deviceString = xxGetInstanceName();
    int language = 0;
    if (language == 0 && strstr(deviceString, "Metal 2"))    language = 'MSL2';
    if (language == 0 && strstr(deviceString, "Metal"))      language = 'MSL1';
    if (language == 0 && strstr(deviceString, "Direct3D 1")) language = 'HLSL';
    if (language == 0 && strstr(deviceString, "Direct3D"))   language = 'hlsl';
    if (language == 0 && strstr(deviceString, "Vulkan"))     language = 'HLSL';
    if (language == 0 && strstr(deviceString, "GL"))         language = 'GLSL';

    uint16_t meshTextureSlot = 0;
    uint16_t vertexTextureSlot = 0;
    uint16_t fragmentTextureSlot = 0;
    std::string shader;

    shader += ShaderOption;
    shader += define("SHADER_NORMAL", mesh->NormalCount);
    shader += define("SHADER_COLOR", mesh->ColorCount);
    shader += define("SHADER_TEXTURE", mesh->TextureCount);
    shader += define("SHADER_LIGHTING", mesh->NormalCount && Lighting ? 1 : 0);
    shader += define("SHADER_SPECULAR", mesh->NormalCount && Lighting && Specular ? 1 : 0);
    shader += define("TEXTURE_BASE", mesh->TextureCount && GetTexture(BASE) ? 1 : 0);
    shader += define("TEXTURE_BUMP", mesh->TextureCount && GetTexture(BUMP) ? 1 : 0);

    struct MaterialSelector s(shader, type, language);

    switch (type)
    {
    case 'mesh':
        shader += define("SHADER_UNIFORM", GetMeshConstantSize(data) / sizeof(xxVector4));
        shader += define("SHADER_BACKFACE_CULLING", BackfaceCulling ? 1 : 0);
        shader += define("SHADER_FRUSTUM_CULLING", FrustumCulling ? 1 : 0);
        shader += define("SHADER_SKINNING", mesh->Skinning ? 1 : 0);
        shader += define("SHADER_OPACITY", Blending ? 1 : 0);
        ShaderDefault(data, s);
        ShaderAttribute(data, s);
        ShaderConstant(data, s);
        ShaderVarying(data, s);
        ShaderMesh(data, s);
        break;
    case 'vert':
        shader += define("SHADER_UNIFORM", GetVertexConstantSize(data) / sizeof(xxVector4));
        shader += define("SHADER_SKINNING", mesh->Skinning ? 1 : 0);
        shader += define("SHADER_OPACITY", Blending ? 1 : 0);
        ShaderDefault(data, s);
        ShaderAttribute(data, s);
        ShaderConstant(data, s);
        ShaderVarying(data, s);
        ShaderVertex(data, s);
        break;
    case 'frag':
        shader += define("SHADER_UNIFORM", GetFragmentConstantSize(data) / sizeof(xxVector4));
        shader += define("SHADER_ALPHATEST", AlphaTest ? 1 : 0);
        ShaderDefault(data, s);
        ShaderConstant(data, s);
        ShaderVarying(data, s);
        ShaderFragment(data, s);
        if (GetTexture(BASE))
        {
            fragmentTextureSlot |= 0b01;
        }
        if (GetTexture(BUMP))
        {
            fragmentTextureSlot |= 0b10;
        }
        break;
    }

    const_cast<uint16_t&>(m_meshTextureSlot) = meshTextureSlot;
    const_cast<uint16_t&>(m_vertexTextureSlot) = vertexTextureSlot;
    const_cast<uint16_t&>(m_fragmentTextureSlot) = fragmentTextureSlot;

    //shader += Shader.empty() ? DefaultShader : Shader;

    return shader;
}
//------------------------------------------------------------------------------
int Material::GetMeshConstantSize(xxDrawData const& data) const
{
    int size = 0;
    UpdateCullingConstant(data, size);
    UpdateWorldViewProjectionConstant(data, size);
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
void Material::ShaderDefault(xxDrawData const& data, struct MaterialSelector& s) const
{
    auto macro = [&](std::string_view name, std::string_view def, bool check = true)
    {
        if (check)
        {
            s.shader += "#ifndef";
            s.shader += ' ';
            s.shader += name;
            s.shader += '\n';
        }
        s.shader += "#define";
        s.shader += ' ';
        s.shader += name;
        s.shader += ' ';
        s.shader += def;
        s.shader += '\n';
        if (check)
        {
            s.shader += "#endif";
            s.shader += '\n';
        }
    };

    if (s.language == 'GLSL')
    {
        macro("SHADER_GLSL", "0");
        macro("SHADER_HLSL", "0");
        macro("SHADER_MSL", "0");
        macro("SHADER_MESH", "0");
        macro("SHADER_VERTEX", "0");
        macro("SHADER_FRAGMENT", "0");
        macro("SHADER_BACKFACE_CULLING", "0");
        macro("SHADER_FRUSTUM_CULLING", "0");
        macro("SHADER_SKINNING", "0");
        macro("SHADER_NORMAL", "0");
        macro("SHADER_COLOR", "1");
        macro("SHADER_TEXTURE", "1");
        macro("SHADER_UNIFORM", "12");
        macro("SHADER_ALPHATEST", "0");
        macro("SHADER_OPACITY", "0");
        macro("SHADER_LIGHTING", "0");
        macro("SHADER_SPECULAR", "0");
        macro("TEXTURE_BASE", "0");
        macro("TEXTURE_BUMP", "0");

        macro("float2", "vec2", false);
        macro("float3", "vec3", false);
        macro("float4", "vec4", false);
        macro("float2x2", "mat2", false);
        macro("float3x3", "mat3", false);
        macro("float4x4", "mat4", false);
        macro("int2", "ivec2", false);
        macro("int3", "ivec3", false);
        macro("int4", "ivec4", false);
    }
    if (s.language == 'GLSL' || s.language == 'MSL1' || s.language == 'MSL2')
    {
        macro("mul(a, b)", "(b * a)", false);
    }
}
//------------------------------------------------------------------------------
void Material::ShaderAttribute(xxDrawData const& data, struct MaterialSelector& s) const
{
    xxMesh* mesh = data.mesh;
    bool skinning = mesh->Skinning;
    int normal = mesh->NormalCount;
    int color = mesh->ColorCount;
    int texture = mesh->TextureCount;

    bool vertexPulling = s.type == 'mesh';
    if (vertexPulling)
    {
        //             GLSL / HLSL / MSL
        s(true,        "struct Attribute"     );
        s(true,        "{"                    );
        s(true,        "float Position[3];"   );
        s(skinning,    "float BoneWeight[3];" );
        s(skinning,    "uint BoneIndices[4];" );
        s(normal > 0,  "float Normal[3];"     );
        s(normal > 1,  "float Tangent[3];"    );
        s(normal > 2,  "float Binormal[3];"   );
        s(color > 0,   "float Color[4];"      );
        s(texture > 0, "float UV0[2];"        );
        s(true,        "};"                   );
    }
    else
    {
        //                 GLSL                               HLSL                                 MSL
        s.GHM(true,        "",                                "struct Attribute",                  "struct Attribute"                              );
        s.GHM(true,        "",                                "{",                                 "{"                                             );
        s.GHM(true,        "attribute vec3 attrPosition;",    "float3 Position : POSITION;",       "float3 Position [[attribute(__COUNTER__)]];"   );
        s.GHM(skinning,    "attribute vec3 attrBoneWeight;",  "float3 BoneWeight : BLENDWEIGHT;",  "float3 BoneWeight [[attribute(__COUNTER__)]];" );
        s.GHM(skinning,    "attribute vec4 attrBoneIndices;", "uint4 BoneIndices : BLENDINDICES;", "uint4 BoneIndices [[attribute(__COUNTER__)]];" );
        s.GHM(normal > 0,  "attribute vec3 attrNormal;",      "float3 Normal : NORMAL;",           "float3 Normal [[attribute(__COUNTER__)]];"     );
        s.GHM(normal > 1,  "attribute vec3 attrTangent;",     "float3 Tangent : TANGENT;",         "float3 Tangent [[attribute(__COUNTER__)]];"    );
        s.GHM(normal > 2,  "attribute vec3 attrBinormal;",    "float3 Binormal : BINORMAL;",       "float3 Binormal [[attribute(__COUNTER__)]];"   );
        s.GHM(color > 0,   "attribute vec4 attrColor;",       "float4 Color : COLOR;",             "float4 Color [[attribute(__COUNTER__)]];"      );
        s.GHM(texture > 0, "attribute vec2 attrUV0;",         "float2 UV0 : TEXCOORD;",            "float2 UV0 [[attribute(__COUNTER__)]];"        );
        s.GHM(true,        "",                                "};",                                "};"                                            );
    }
}
//------------------------------------------------------------------------------
void Material::ShaderConstant(xxDrawData const& data, struct MaterialSelector& s) const
{
    bool mesh = s.type == 'mesh';
    bool vert = s.type == 'vert';
    bool frag = s.type == 'frag';
    bool base = s.type == 'frag' && GetTexture(BASE) != nullptr;
    bool bump = s.type == 'frag' && GetTexture(BUMP) != nullptr;

    if (mesh)
    {
        s(true, "struct Meshlet"         );
        s(true, "{"                      );
        s(true, "uint VertexOffset;"     );
        s(true, "uint TriangleOffset;"   );
        s(true, "uint VertexCount;"      );
        s(true, "uint TriangleCount;"    );
        s(true, "float4 CenterRadius;"   );
        s(true, "float4 ConeApex;"       );
        s(true, "float4 ConeAxisCutoff;" );
        s(true, "};"                     );
    }

    //           GLSL / HLSL                                  HLSL10                               MSL                                           MSL Argument
    s.HHMM(true, "",                                          "",                                  "",                                           "struct Uniform"                         );
    s.HHMM(true, "",                                          "",                                  "",                                           "{"                                      );
    s.HHMM(mesh, "",                                          "",                                  "struct MeshBuffer",                          ""                                       );
    s.HHMM(mesh, "",                                          "",                                  "{",                                          ""                                       );
    s.HHMM(mesh, "",                                          "",                                  "device Attribute* Vertices [[buffer(0)]];",  "device Attribute* Vertices [[id(0)]];"  );
    s.HHMM(mesh, "",                                          "",                                  "device Meshlet* Meshlets [[buffer(1)]];",    "device Meshlet* Meshlets [[id(1)]];"    );
    s.HHMM(mesh, "",                                          "",                                  "device uint* VertexIndices [[buffer(2)]];",  "device uint* VertexIndices [[id(2)]];"  );
    s.HHMM(mesh, "",                                          "",                                  "device uint* TriangeIndices [[buffer(3)]];", "device uint* TriangeIndices [[id(3)]];" );
    s.HHMM(mesh, "",                                          "",                                  "};",                                         ""                                       );
    s.HHMM(true, "",                                          "",                                  "struct Uniform",                             ""                                       );
    s.HHMM(true, "",                                          "",                                  "{",                                          ""                                       );
    s.HHMM(true, "#if SHADER_UNIFORM",                        "#if SHADER_UNIFORM",                "#if SHADER_UNIFORM",                         ""                                       );
    s.HHMM(mesh, "uniform float4 uniBuffer[SHADER_UNIFORM];", "float4 uniBuffer[SHADER_UNIFORM];", "float4 Buffer[SHADER_UNIFORM];",             "device float4* Buffer [[id(30)]];"      );
    s.HHMM(vert, "uniform float4 uniBuffer[SHADER_UNIFORM];", "float4 uniBuffer[SHADER_UNIFORM];", "float4 Buffer[SHADER_UNIFORM];",             "device float4* Buffer [[id(0)]];"       );
    s.HHMM(frag, "uniform float4 uniBuffer[SHADER_UNIFORM];", "float4 uniBuffer[SHADER_UNIFORM];", "float4 Buffer[SHADER_UNIFORM];",             "device float4* Buffer [[id(1)]];"       );
    s.HHMM(true, "#endif",                                    "#endif",                            "#endif",                                     ""                                       );
    s.HHMM(true, "",                                          "",                                  "};",                                         ""                                       );
    s.HHMM(true, "",                                          "",                                  "struct Sampler",                             ""                                       );
    s.HHMM(true, "",                                          "",                                  "{",                                          ""                                       );
    s.HHMM(base, "",                                          "Texture2D<float4> Base;",           "texture2d<float> Base [[texture(0)]];",      "texture2d<float> Base [[id(4)]];"       );
    s.HHMM(bump, "",                                          "Texture2D<float4> Bump;",           "texture2d<float> Bump [[texture(1)]];",      "texture2d<float> Bump [[id(5)]];"       );
    s.HHMM(base, "uniform sampler2D BaseSampler;",            "sampler BaseSampler;",              "sampler BaseSampler [[sampler(0)]];",        "sampler BaseSampler [[id(18)]];"        );
    s.HHMM(bump, "uniform sampler2D BumpSampler;",            "sampler BumpSampler;",              "sampler BumpSampler [[sampler(1)]];",        "sampler BumpSampler [[id(19)]];"        );
    s.HHMM(true, "",                                          "",                                  "};",                                         "};"                                     );
    s.HHMM(frag, "",                                          "",                                  "",                                           "struct Sampler {};"                     );
}
//------------------------------------------------------------------------------
void Material::ShaderVarying(xxDrawData const& data, struct MaterialSelector& s) const
{
    xxMesh* mesh = data.mesh;
    int normal = mesh->NormalCount;
    int color = mesh->ColorCount;
    int texture = mesh->TextureCount;

    //                       GLSL                               HLSL                                 MSL
    s.GHM(true,              "",                                "struct Varying",                    "struct Varying"                );
    s.GHM(true,              "",                                "{",                                 "{"                             );
    s.GHM(true,              "",                                "float4 Position : SV_POSITION",     "float4 Position [[position]];" );
    s.GHM(color || Lighting, "varying vec4 varyColor;",         "float4 Color : COLOR;",             "float4 Color;"                 );
    s.GHM(texture > 0,       "varying vec2 varyUV0;",           "float2 UV0 : TEXCOORD0;",           "float2 UV0;"                   );
    s.GHM(Specular,          "varying vec3 varyWorldPosition;", "float3 WorldPosition : TEXCOORD4;", "float3 WorldPosition;"         );
    s.GHM(normal > 0,        "varying vec3 varyWorldNormal;",   "float3 WorldNormal : TEXCOORD5;",   "float3 WorldNormal;"           );
    s.GHM(normal > 1,        "varying vec3 varyWorldTangent;",  "float3 WorldTangent : TEXCOORD6;",  "float3 WorldTangent;"          );
    s.GHM(normal > 2,        "varying vec3 varyWorldBinormal;", "float3 WorldBinormal : TEXCOORD7;", "float3 WorldBinormal;"         );
    s.GHM(true,              "",                                "};",                                "};"                            );
}
//------------------------------------------------------------------------------
void Material::ShaderMesh(xxDrawData const& data, struct MaterialSelector& s) const
{
    xxMesh* mesh = data.mesh;
    bool skinning = mesh->Skinning;
    int normal = mesh->NormalCount;
    int color = mesh->ColorCount;
    int texture = mesh->TextureCount;
    int size = 0;

    //             HLSL                                MSL                                                        MSL Arugment
    s.HMM(true,    "[outputtopology(\"triangle\")]",   "[[mesh]]",                                                "[[mesh]]"                                                );
    s.HMM(true,    "[numthreads(128, 1, 1)",           "",                                                        ""                                                        );
    s.HMM(true,    "void Main",                        "void Main",                                               "void Main"                                               );
    s.HMM(true,    "(",                                "(",                                                       "("                                                       );
    s.HMM(true,    "",                                 "constant Uniform& uni [[buffer(30)]],",                   "constant Uniform& uni [[buffer(0)]],"                    );
    s.HMM(true,    "",                                 "MeshBuffer mb,",                                          ""                                                        );
    s.HMM(true,    "uint gtid : SV_GroupThreadID,",    "uint gtid [[thread_position_in_threadgroup]],",           "uint gtid [[thread_position_in_threadgroup]],"           );
    s.HMM(true,    "uint gid : SV_GroupID,",           "uint gid [[threadgroup_position_in_grid]],",              "uint gid [[threadgroup_position_in_grid]],"              );
    s.HMM(true,    "out indices uint3 triangles[64],", "mesh<Varying, void, 64, 128, topology::triangle> output", "mesh<Varying, void, 64, 128, topology::triangle> output" );
    s.HMM(true,    "out vertices Varying vary[128]",   "",                                                        ""                                                        );
    s.HMM(true,    ")",                                ")",                                                       ")"                                                       );
    s.HMM(true,    "{",                                "{",                                                       "{"                                                       );
    s.HMM(true,    "int uniIndex = 0;",                "int uniIndex = 0;",                                       "int uniIndex = 0;"                                       );
    s.HMM(true,    "",                                 "auto uniBuffer = uni.Buffer;",                            "auto uniBuffer = uni.Buffer;"                            );
    s.HMM(true,    "",                                 "device Attribute* Vertices = mb.Vertices;",               "device Attribute* Vertices = uni.Vertices;"              );
    s.HMM(true,    "",                                 "device uint* VertexIndices = mb.VertexIndices;",          "device uint* VertexIndices = uni.VertexIndices;"         );
    s.HMM(true,    "",                                 "device uint* TriangeIndices = mb.TriangeIndices;",        "device uint* TriangeIndices = uni.TriangeIndices;"       );
    s.HMM(true,    "Meshlet& m = Meshlets[gid];",      "device Meshlet& m = mb.Meshlets[gid];",                   "device Meshlet& m = uni.Meshlets[gid];"                  );

    UpdateCullingConstant(data, size, nullptr, &s);

    s(true,        "if (gtid == 0)"                                                                                                     );
    s(true,        "{"                                                                                                                  );
    s.HM(true,     "SetMeshOutputCounts(m.VertexCount, m.TriangleCount);", ""                                                           );
    s.HM(true,     "", "output.set_primitive_count(m.TriangleCount);"                                                                   );
    s(true,        "}"                                                                                                                  );
    s(true,        "if (gtid < m.TriangleCount)"                                                                                        );
    s(true,        "{"                                                                                                                  );
    s(true,        "uint index = 3 * gtid;"                                                                                             );
    s(true,        "uint packed = TriangeIndices[m.TriangleOffset + gtid];"                                                             );
    s.HM(true,     "triangles[gtid].x = (packed >>  0) & 0xFF);", ""                                                                    );
    s.HM(true,     "triangles[gtid].y = (packed >>  8) & 0xFF);", ""                                                                    );
    s.HM(true,     "triangles[gtid].z = (packed >> 16) & 0xFF);", ""                                                                    );
    s.HM(true,     "", "output.set_index(index + 0, (packed >>  0) & 0xFF);"                                                            );
    s.HM(true,     "", "output.set_index(index + 1, (packed >>  8) & 0xFF);"                                                            );
    s.HM(true,     "", "output.set_index(index + 2, (packed >> 16) & 0xFF);"                                                            );
    s(true,        "}"                                                                                                                  );
    s(true,        "if (gtid >= m.VertexCount) return;"                                                                                 );
    s(true,        "uint vertexIndex = VertexIndices[m.VertexOffset + gtid];"                                                           );
    s(true,        "Attribute attr = Vertices[vertexIndex];"                                                                            );
    s(true,        "float3 attrPosition = float3(attr.Position[0], attr.Position[1], attr.Position[2]);"                                );
    s(skinning,    "float3 attrBoneWeight = float3(attr.BoneWeight[0], attr.BoneWeight[1], attr.BoneWeight[2]);"                        );
    s(skinning,    "uint4 attrBoneIndices = uint4(attr.BoneIndices[0], attr.BoneIndices[1], attr.BoneIndices[2], attr.BoneIndices[3]);" );
    s(color,       "float4 attrColor = float4(attr.Color[0], attr.Color[1], attr.Color[2], attr.Color[3]);"                             );
    s(texture > 0, "float2 attrUV0 = float2(attr.UV0[0], attr.UV0[1]);"                                                                 );
    s(normal > 0,  "float3 attrNormal = float3(attr.Normal[0], attr.Normal[1], attr.Normal[2]);"                                        );
    s(normal > 1,  "float3 attrTangent = float3(attr.Tangent[0], attr.Tangent[1], attr.Tangent[2]);"                                    );
    s(normal > 2,  "float3 attrBinormal = float3(attr.Binormal[0], attr.Binormal[1], attr.Binormal[2]);"                                );
    s(true,        "float4 color = float4(1.0, 1.0, 1.0, 1.0);"                                                                         );
    s(color,       "color = attrColor;"                                                                                                 );

    UpdateWorldViewProjectionConstant(data, size, nullptr, &s);
    UpdateBlendingConstant(data, size, nullptr, &s);
    UpdateLightingConstant(data, size, nullptr, &s);

    //                           HLSL                                             MSL
    s.HM(true,                   "",                                              "Varying vary;"                           );
    s.HM(true,                   "vary[gtid].Position = screenPosition;",         "vary.Position = screenPosition;"         );
    s.HM(Lighting || color,      "vary[gtid].Color = color;",                     "vary.Color = color;"                     );
    s.HM(texture > 0,            "vary[gtid].UV0 = attrUV0;",                     "vary.UV0 = attrUV0;"                     );
    s.HM(Lighting && Specular,   "vary[gtid].WorldPosition = worldPosition.xyz;", "vary.WorldPosition = worldPosition.xyz;" );
    s.HM(Lighting && normal > 0, "vary[gtid].WorldNormal = worldNormal;",         "vary.WorldNormal = worldNormal;"         );
    s.HM(Lighting && normal > 1, "vary[gtid].WorldTangent = worldTangent;",       "vary.WorldTangent = worldTangent;"       );
    s.HM(Lighting && normal > 2, "vary[gtid].WorldBinormal = worldBinormal;",     "vary.WorldBinormal = worldBinormal;"     );
    s.HM(true,                   "",                                              "output.set_vertex(gtid, vary);"          );
    s.HM(true,                   "}",                                             "}"                                       );
}
//------------------------------------------------------------------------------
void Material::ShaderVertex(xxDrawData const& data, struct MaterialSelector& s) const
{
    xxMesh* mesh = data.mesh;
    bool skinning = mesh->Skinning;
    int normal = mesh->NormalCount;
    int color = mesh->ColorCount;
    int texture = mesh->TextureCount;
    int size = 0;

    //          GLSL                       HLSL                            MSL
    s.GHM(true, "",                        "",                             "vertex"                                    );
    s.GHM(true, "void main()",             "Varying Main(Attribute attr)", "Varying Main(Attribute attr [[stage_in]]," );
    s.GHM(true, "",                        "",                             "constant Uniform& uni [[buffer(0)]])"      );
    s.GHM(true, "{",                       "{",                            "{"                                         );
    s.GHM(true, "",                        "",                             "auto uniBuffer = uni.Buffer;"              );
    s.GHM(true, "int uniIndex = 0;",       "int uniIndex = 0;",            "int uniIndex = 0;"                         );
    s.GHM(true, "vec4 color = vec4(1.0);", "float4 color = float4(1.0);",  "float4 color = 1.0;"                       );

    //                GLSL                  HLSL / MSL
    s.GH(true,        "",                   "float3 attrPosition = attr.Position;"      );
    s.GH(skinning,    "",                   "float3 attrBoneWeight = attr.BoneWeight;"  );
    s.GH(skinning,    "",                   "uint4 attrBoneIndices = attr.BoneIndices;" );
    s.GH(color,       "",                   "float4 attrColor = attr.Color;"            );
    s.GH(texture > 0, "",                   "float2 attrUV0 = attr.UV0;"                );
    s.GH(normal > 0,  "",                   "float3 attrNormal = attr.Normal;"          );
    s.GH(normal > 1,  "",                   "float3 attrTangent = attr.Tangent;"        );
    s.GH(normal > 2,  "",                   "float3 attrBinormal = attr.Binormal;"      );
    s.GH(color,       "color = attrColor;", "color = attrColor;"                        );

    UpdateWorldViewProjectionConstant(data, size, nullptr, &s);
    UpdateSkinningConstant(data, size, nullptr, &s);
    UpdateBlendingConstant(data, size, nullptr, &s);
    UpdateLightingConstant(data, size, nullptr, &s);

    //                           GLSL                                      HLSL / MSL
    s.GH(true,                   "",                                       "Varying vary;"                           );
    s.GH(true,                   "gl_Position = screenPosition;",          "vary.Position = screenPosition;"         );
    s.GH(Lighting || color,      "varyColor = color;",                     "vary.Color = color;"                     );
    s.GH(texture > 0,            "varyUV0 = attrUV0;",                     "vary.UV0 = attrUV0;"                     );
    s.GH(Lighting && Specular,   "varyWorldPosition = worldPosition.xyz;", "vary.WorldPosition = worldPosition.xyz;" );
    s.GH(Lighting && normal > 0, "varyWorldNormal = worldNormal;",         "vary.WorldNormal = worldNormal;"         );
    s.GH(Lighting && normal > 1, "varyWorldTangent = worldTangent;",       "vary.WorldTangent = worldTangent;"       );
    s.GH(Lighting && normal > 2, "varyWorldBinormal = worldBinormal;",     "vary.WorldBinormal = worldBinormal;"     );
    s.GH(true,                   "",                                       "return vary;"                            );
    s.GH(true,                   "}",                                      "}"                                       );
}
//------------------------------------------------------------------------------
void Material::ShaderFragment(xxDrawData const& data, struct MaterialSelector& s) const
{
    xxMesh* mesh = data.mesh;
    int normal = mesh->NormalCount;
    int color = mesh->ColorCount;
    int texture = mesh->TextureCount;
    int size = 0;

    bool base = GetTexture(BASE) != nullptr;
    bool bump = GetTexture(BUMP) != nullptr;

    //          GLSL                       HLSL                                  MSL
    s.GHM(true, "",                        "",                                   "fragment"                               );
    s.GHM(true, "void main()",             "float4 Main(Varying vary) : COLOR0", "float4 Main(Varying vary [[stage_in]]," );
    s.GHM(true, "",                        "",                                   "constant Uniform& uni [[buffer(0)]],"   );
    s.GHM(true, "",                        "",                                   "Sampler sam)"                           );
    s.GHM(true, "{",                       "{",                                  "{"                                      );
    s.GHM(true, "",                        "",                                   "#if SHADER_UNIFORM"                     );
    s.GHM(true, "",                        "",                                   "auto uniBuffer = uni.Buffer;"           );
    s.GHM(true, "",                        "",                                   "#endif"                                 );
    s.GHM(true, "int uniIndex = 0;",       "int uniIndex = 0;",                  "int uniIndex = 0;"                      );
    s.GHM(true, "vec4 color = vec4(1.0);", "float4 color = float4(1.0);",        "float4 color = 1.0;"                    );
    s.GHM(bump, "vec4 bump = vec4(0.0);",  "float4 bump = float4(0.0);",         "float4 bump = 0.0;"                     );

    //                           GLSL                  HLSL / MSL
    s.GH(Lighting || color,      "",                   "float4 varyColor = vary.Color;"                 );
    s.GH(texture > 0,            "",                   "float2 varyUV0 = vary.UV0;"                     );
    s.GH(Lighting && Specular,   "",                   "float3 varyWorldPosition = vary.WorldPosition;" );
    s.GH(Lighting && normal > 0, "",                   "float3 varyWorldNormal = vary.WorldNormal;"     );
    s.GH(Lighting && normal > 1, "",                   "float3 varyWorldTangent = vary.WorldTangent;"   );
    s.GH(Lighting && normal > 2, "",                   "float3 varyWorldBinormal = vary.WorldBinormal;" );
    s.GH(Lighting || color,      "color = varyColor;", "color = varyColor;"                             );

    //           GLSL HLSL MSL                                    MSL Argument
    s.GHMM(base, "",  "",  "auto Base = sam.Base;",               "auto Base = uni.Base;"               );
    s.GHMM(bump, "",  "",  "auto Bump = sam.Bump;",               "auto Bump = uni.Bump;"               );
    s.GHMM(base, "",  "",  "auto BaseSampler = sam.BaseSampler;", "auto BaseSampler = uni.BaseSampler;" );
    s.GHMM(bump, "",  "",  "auto BumpSampler = sam.BumpSampler;", "auto BumpSampler = uni.BumpSampler;" );

    //           GLSL                                         HLSL                                     HLSL10                                         MSL
    s.GHHM(base, "color *= texture2D(BaseSampler, varyUV0);", "color *= tex2D(BaseSampler, varyUV0);", "color *= Base.Sample(BaseSampler, varyUV0);", "color *= Base.sample(BaseSampler, varyUV0);" );
    s.GHHM(bump, "bump = texture2D(BumpSampler, varyUV0);",   "bump = tex2D(BumpSampler, varyUV0);",   "bump = Bump.Sample(BumpSampler, varyUV0);",   "bump = Bump.sample(BumpSampler, varyUV0);"   );
    s.GHHM(bump, "bump = bump * 2.0 - 1.0;",                  "bump = bump * 2.0 - 1.0;",              "bump = bump * 2.0 - 1.0;",                    "bump = bump * 2.0 - 1.0;"                    );

    UpdateAlphaTestingConstant(data, size, nullptr, &s);
    UpdateLightingConstant(data, size, nullptr, &s);

    //         GLSL                     HLSL / MSL
    s.GH(true, "gl_FragColor = color;", "return color;" );
    s.GH(true, "}",                     "}"             );
}
//------------------------------------------------------------------------------
void Material::UpdateAlphaTestingConstant(xxDrawData const& data, int& size, xxVector4** pointer, struct MaterialSelector* s) const
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
    if (s)
    {
        (*s)(true,     "float alphaRef = uniBuffer[uniIndex++].x;"    );
        (*s)(true,     "if (color.a < alphaRef)"                      );
        (*s)(true,     "{"                                            );
        (*s).GHM(true, "discard;", "clip(-1);", "discard_fragment();" );
        (*s)(true,     "}"                                            );
    }
}
//------------------------------------------------------------------------------
void Material::UpdateBlendingConstant(xxDrawData const& data, int& size, xxVector4** pointer, struct MaterialSelector* s) const
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
    if (s)
    {
        (*s)(true, "color.a = uniBuffer[uniIndex++].x;");
    }
}
//------------------------------------------------------------------------------
void Material::UpdateCullingConstant(xxDrawData const& data, int& size, xxVector4** pointer, struct MaterialSelector* s) const
{
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
    if (s)
    {
        (*s)(true,            "uint visible = 1;"                                                                                                                  );
        (*s).GHM(true,        "if (subgroupElect())", "", ""                                                                                                       );
        (*s).GHM(true,        "", "if (WaveIsFirstLane())", ""                                                                                                     );
        (*s).GHM(true,        "", "", "if (simd_is_first())"                                                                                                       );
        (*s)(true,            "{"                                                                                                                                  );
        (*s)(true,            "float4x4 world = float4x4(uniBuffer[uniIndex + 12], uniBuffer[uniIndex + 13], uniBuffer[uniIndex + 14], uniBuffer[uniIndex + 15]);" );
        (*s)(BackfaceCulling, "if (visible)"                                                                                                                       );
        (*s)(BackfaceCulling, "{"                                                                                                                                  );
        (*s)(BackfaceCulling, "float4 coneApex = mul(m.ConeApex, world);"                                                                                          );
        (*s)(BackfaceCulling, "float4 coneAxisCutoff = mul(m.ConeAxisCutoff, world);"                                                                              );
        (*s)(BackfaceCulling, "float c = m.ConeAxisCutoff.w;"                                                                                                      );
        (*s)(BackfaceCulling, "float d = dot(coneAxisCutoff.xyz, normalize(coneApex.xyz - uniBuffer[uniIndex + 1].xyz));"                                          );
        (*s)(BackfaceCulling, "if (d >= c) visible = 0;"                                                                                                           );
        (*s)(BackfaceCulling, "}"                                                                                                                                  );
        (*s)(FrustumCulling,  "if (visible)"                                                                                                                       );
        (*s)(FrustumCulling,  "{"                                                                                                                                  );
        (*s)(FrustumCulling,  "float4 centerRadius = mul(m.CenterRadius, world);"                                                                                  );
        (*s)(FrustumCulling,  "float r = -m.CenterRadius.w;"                                                                                                       );
        (*s)(FrustumCulling,  "float d0 = dot(uniBuffer[uniIndex + 0].xyz, centerRadius.xyz - uniBuffer[uniIndex + 1].xyz);"                                       );
        (*s)(FrustumCulling,  "float d1 = dot(uniBuffer[uniIndex + 2].xyz, centerRadius.xyz - uniBuffer[uniIndex + 3].xyz);"                                       );
        (*s)(FrustumCulling,  "float d2 = dot(uniBuffer[uniIndex + 4].xyz, centerRadius.xyz - uniBuffer[uniIndex + 5].xyz);"                                       );
        (*s)(FrustumCulling,  "float d3 = dot(uniBuffer[uniIndex + 6].xyz, centerRadius.xyz - uniBuffer[uniIndex + 7].xyz);"                                       );
        (*s)(FrustumCulling,  "float d4 = dot(uniBuffer[uniIndex + 8].xyz, centerRadius.xyz - uniBuffer[uniIndex + 9].xyz);"                                       );
        (*s)(FrustumCulling,  "float d5 = dot(uniBuffer[uniIndex + 10].xyz, centerRadius.xyz - uniBuffer[uniIndex + 11].xyz);"                                     );
        (*s)(FrustumCulling,  "if (d0 < r || d1 < r || d2 < r || d3 < r || d4 < r || d5 < r) visible = 0;"                                                         );
        (*s)(FrustumCulling,  "}"                                                                                                                                  );
        (*s)(true,            "}"                                                                                                                                  );
        (*s).GHM(true,        "visible = subgroupBroadcastFirst(visible);", "", ""                                                                                 );
        (*s).GHM(true,        "", "visible = WaveReadLaneFirst(visible);", ""                                                                                      );
        (*s).GHM(true,        "", "", "visible = simd_broadcast_first(visible);"                                                                                   );
        (*s)(true,            "if (visible == 0) return;"                                                                                                          );
        (*s)(true,            "uniIndex += 12;"                                                                                                                    );
    }
}
//------------------------------------------------------------------------------
void Material::UpdateLightingConstant(xxDrawData const& data, int& size, xxVector4** pointer, struct MaterialSelector* s) const
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
    if (s)
    {
        int normal = data.mesh->NormalCount;

        bool mesh = s->type == 'mesh';
        bool vert = s->type == 'vert';
        bool frag = s->type == 'frag';

        bool bump = GetTexture(BUMP) != nullptr;

        (*s)((mesh || vert) && normal > 0, "float3 worldNormal = normalize(mul(float4(attrNormal, 1.0), world).xyz);"     );
        (*s)((mesh || vert) && normal > 1, "float3 worldTangent = normalize(mul(float4(attrTangent, 1.0), world).xyz);"   );
        (*s)((mesh || vert) && normal > 2, "float3 worldBinormal = normalize(mul(float4(attrBinormal, 1.0), world).xyz);" );

        (*s)(true, "float3 cameraPosition = uniBuffer[uniIndex++].xyz;" );
        (*s)(true, "float3 lightDirection = uniBuffer[uniIndex++].xyz;" );
        (*s)(true, "float3 lightColor = uniBuffer[uniIndex++].xyz;"     );
        (*s)(true, "float3 ambientColor = uniBuffer[uniIndex++].xyz;"   );
        (*s)(true, "float3 diffuseColor = uniBuffer[uniIndex++].xyz;"   );
        (*s)(true, "float3 emissiveColor = uniBuffer[uniIndex++].xyz;"  );
        (*s)(true, "float3 specularColor = uniBuffer[uniIndex].xyz;"    );
        (*s)(true, "float specularHighlight = uniBuffer[uniIndex++].w;" );

        (*s)(true,             "float3 L = lightDirection;"                                         );
        (*s)(mesh || vert,     "float3 N = worldNormal;"                                            );
        (*s)(frag,             "float3 N = varyWorldNormal;"                                        );
        (*s)(frag && bump,     "N.z = dot(varyWorldNormal, bump.xyz);"                              );
        (*s)(frag && bump,     "N.x = dot(varyWorldTangent, bump.xyz);"                             );
        (*s)(frag && bump,     "N.y = dot(varyWorldBinormal, bump.xyz);"                            );
        (*s)(frag == bump,     "float lambert = dot(N, L);"                                         );
        (*s)(frag == bump,     "color.rgb *= (ambientColor + diffuseColor * lightColor * lambert);" );
        (*s)(frag == bump,     "color.rgb += emissiveColor;"                                        );
        (*s)(frag && Specular, "float3 V = normalize(cameraPosition - varyWorldPosition);"          );
        (*s)(frag && Specular, "float3 H = normalize(V + L);"                                       );
        (*s)(frag && Specular, "float phong = pow(max(dot(N, H), 0.0001), specularHighlight);"      );
        (*s)(frag && Specular, "color.rgb = color.rgb + specularColor * phong;"                     );
    }
}
//------------------------------------------------------------------------------
void Material::UpdateSkinningConstant(xxDrawData const& data, int& size, xxVector4** pointer, struct MaterialSelector* s) const
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
    if (s)
    {
        (*s)(true, "float4 zero4 = float4(0.0, 0.0, 0.0, 0.0);"                                                                                                        );
        (*s)(true, "float4 boneWeight = float4(attrBoneWeight, 1.0 - attrBoneWeight.x - attrBoneWeight.y - attrBoneWeight.z);"                                         );
        (*s)(true, "int4 boneIndices = int4(attrBoneIndices);"                                                                                                         );
        (*s)(true, "world  = float4x4(uniBuffer[boneIndices.x * 3 + 12], uniBuffer[boneIndices.x * 3 + 13], uniBuffer[boneIndices.x * 3 + 14], zero4) * boneWeight.x;" );
        (*s)(true, "world += float4x4(uniBuffer[boneIndices.y * 3 + 12], uniBuffer[boneIndices.y * 3 + 13], uniBuffer[boneIndices.y * 3 + 14], zero4) * boneWeight.y;" );
        (*s)(true, "world += float4x4(uniBuffer[boneIndices.z * 3 + 12], uniBuffer[boneIndices.z * 3 + 13], uniBuffer[boneIndices.z * 3 + 14], zero4) * boneWeight.z;" );
        (*s)(true, "world += float4x4(uniBuffer[boneIndices.w * 3 + 12], uniBuffer[boneIndices.w * 3 + 13], uniBuffer[boneIndices.w * 3 + 14], zero4) * boneWeight.w;" );
        (*s)(true, "world = transpose(world);"                                                                                                                         );
        (*s)(true, "world[3][3] = 1.0;"                                                                                                                                );
        (*s)(true, "worldPosition = mul(float4(attrPosition, 1.0), world);"                                                                                            );
        (*s)(true, "screenPosition = mul(mul(worldPosition, view), projection);"                                                                                       );
        (*s)(true, "uniIndex += 75 * 3;"                                                                                                                               );
    }
}
//------------------------------------------------------------------------------
void Material::UpdateWorldViewProjectionConstant(xxDrawData const& data, int& size, xxVector4** pointer, struct MaterialSelector* s) const
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
    if (s)
    {
        (*s)(true, "float4x4 world = float4x4(uniBuffer[uniIndex + 0], uniBuffer[uniIndex + 1], uniBuffer[uniIndex + 2], uniBuffer[uniIndex + 3]);"        );
        (*s)(true, "float4x4 view = float4x4(uniBuffer[uniIndex + 4], uniBuffer[uniIndex + 5], uniBuffer[uniIndex + 6], uniBuffer[uniIndex + 7]);"         );
        (*s)(true, "float4x4 projection = float4x4(uniBuffer[uniIndex + 8], uniBuffer[uniIndex + 9], uniBuffer[uniIndex + 10], uniBuffer[uniIndex + 11]);" );
        (*s)(true, "float4 worldPosition = mul(float4(attrPosition, 1.0), world);"                                                                         );
        (*s)(true, "float4 screenPosition = mul(mul(worldPosition, view), projection);"                                                                    );
        (*s)(true, "uniIndex += 12;"                                                                                                                       );
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
