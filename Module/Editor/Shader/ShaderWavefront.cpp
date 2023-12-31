//==============================================================================
// Minamoto : ShaderWavefront Source
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "ShaderWavefront.h"

//==============================================================================
//  Shader
//==============================================================================
static char const* const IllumShaderCode =
R"(#if SHADER_GLSL
#define float2 vec2
#define float3 vec3
#define float4 vec4
#define float4x4 mat4
#endif

#if SHADER_GLSL || SHADER_MSL
#define mul(a, b) (b * a)
#endif

#if SHADER_GLSL || SHADER_HLSL
uniform float4 uniBuffer[12];
#elif SHADER_MSL
struct Uniform
{
#if SHADER_MSL_ARGUMENT
    device float4* Buffer [[id(0)]];
#else
    float4 Buffer[12];
#endif
};
#endif

#if SHADER_GLSL
attribute vec3 attrPosition;
attribute vec3 attrNormal;
attribute vec2 attrUV0;
#elif SHADER_HLSL
struct Attribute
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV0      : TEXCOORD;
};
#elif SHADER_MSL
struct Attribute
{
    float3 Position [[attribute(0)]];
    float3 Normal   [[attribute(1)]];
    float2 UV0      [[attribute(2)]];
};
#endif

#if SHADER_GLSL
varying vec2 varyUV0;
#elif SHADER_HLSL
struct Varying
{
    float4 Position : SV_POSITION;
    float2 UV0      : TEXCOORD0;
};
#elif SHADER_MSL
struct Varying
{
    float4 Position [[position]];
    float2 UV0;
};
#endif

#if SHADER_GLSL || SHADER_HLSL
uniform sampler2D samDiffuse;
#elif SHADER_MSL
struct Sampler
{
#if SHADER_MSL_ARGUMENT
    texture2d<float> Diffuse    [[id(4)]];
    sampler DiffuseSampler      [[id(18)]];
#else
    texture2d<float> Diffuse    [[texture(0)]];
    sampler DiffuseSampler      [[sampler(0)]];
#endif
};
#endif

#if SHADER_VERTEX
#if SHADER_GLSL
void main()
#elif SHADER_HLSL
Varying Main(Attribute attr)
#elif SHADER_MSL
vertex Varying Main(Attribute attr [[stage_in]],
                    constant Uniform& uni [[buffer(0)]])
#endif
{
#if SHADER_HLSL || SHADER_MSL
    float3 attrPosition = attr.Position;
    float2 attrUV0 = attr.UV0;
#endif
#if SHADER_MSL
    auto uniBuffer = uni.Buffer;
#endif
    float4x4 world = float4x4(uniBuffer[0], uniBuffer[1], uniBuffer[2], uniBuffer[3]);
    float4x4 view = float4x4(uniBuffer[4], uniBuffer[5], uniBuffer[6], uniBuffer[7]);
    float4x4 projection = float4x4(uniBuffer[8], uniBuffer[9], uniBuffer[10], uniBuffer[11]);
    float4 vpos = mul(mul(mul(float4(attrPosition, 1.0), world), view), projection);
#if SHADER_GLSL
    gl_Position = vpos;
    varyUV0 = attrUV0;
#elif SHADER_HLSL || SHADER_MSL
    Varying vary;
    vary.Position = vpos;
    vary.UV0 = attrUV0;
    return vary;
#endif
}
#endif

#if SHADER_FRAGMENT
#if SHADER_GLSL
void main()
#elif SHADER_HLSL
float4 Main(Varying vary) : COLOR0
#elif SHADER_MSL
fragment float4 Main(Varying vary [[stage_in]],
#if SHADER_MSL_ARGUMENT
                     constant Sampler& sam [[buffer(0)]])
#else
                     Sampler sam)
#endif
#endif
{
#if SHADER_GLSL
    gl_FragColor = texture2D(samDiffuse, varyUV0);
#elif SHADER_HLSL
    return tex2D(samDiffuse, vary.UV0);
#elif SHADER_MSL
    return sam.Diffuse.sample(sam.DiffuseSampler, vary.UV0);
#endif
}
#endif)";
//-----------------------------------------------------------------------------
char const* ShaderWavefront::Illumination(int type)
{
    return IllumShaderCode;
}
//==============================================================================
