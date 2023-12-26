//==============================================================================
// Minamoto : ShaderWavefront Source
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <xxGraphic/xxGraphic.h>
#include <xxGraphic/xxGraphicD3D5.h>
#include <xxGraphic/xxGraphicD3D6.h>
#include <xxGraphic/xxGraphicD3D7.h>
#include <xxGraphic/xxGraphicD3D8.h>
#include <xxGraphic/xxGraphicD3D8PS.h>
#include <xxGraphic/xxGraphicD3D9.h>
#include <xxGraphic/xxGraphicD3D9PS.h>
#include <xxGraphic/xxGraphicD3D9Ex.h>
#include <xxGraphic/xxGraphicD3D9On12.h>
#include <xxGraphic/xxGraphicD3D10.h>
#include <xxGraphic/xxGraphicD3D10_1.h>
#include <xxGraphic/xxGraphicD3D11.h>
#include <xxGraphic/xxGraphicD3D11On12.h>
#include <xxGraphic/xxGraphicD3D12.h>
#include <xxGraphic/xxGraphicGlide.h>
#include <xxGraphic/xxGraphicGLES2.h>
#include <xxGraphic/xxGraphicGLES3.h>
#include <xxGraphic/xxGraphicGLES31.h>
#include <xxGraphic/xxGraphicGLES32.h>
#include <xxGraphic/xxGraphicMetal.h>
#include <xxGraphic/xxGraphicMetal2.h>
#include <xxGraphic/xxGraphicNULL.h>
#include <xxGraphic/xxGraphicVulkan.h>
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
#elif SHADER_METAL
#include <metal_stdlib>
using namespace metal;
#endif

#if SHADER_GLSL
uniform vec4 buf[12];
#elif SHADER_METAL
struct Uniform
{
#if __METAL_USE_ARGUMENT__ == 0
    float4x4 matrix[3];
#else
    device float4x4* matrix [[id(0)]];
#endif
};
#endif

#if SHADER_GLSL
attribute vec3 position;
attribute vec3 normal;
attribute vec2 uv;
#elif SHADER_METAL
struct Attribute
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 uv       [[attribute(2)]];
};
#endif

#if SHADER_GLSL
varying vec2 varyUV;
#elif SHADER_METAL
struct Varying
{
    float4 varyPosition [[position]];
    float2 varyUV;
};
#endif

#if SHADER_GLSL
uniform sampler2D tex;
#elif SHADER_METAL
struct TextureSampler
{
#if __METAL_USE_ARGUMENT__ == 0
    texture2d<float> tex [[texture(0)]];
    sampler sam          [[sampler(0)]];
#else
    texture2d<float> tex [[id(4)]];
    sampler sam          [[id(18)]];
#endif
};
#endif

#if SHADER_VERTEX
#if SHADER_GLSL
void main()
#elif SHADER_METAL
vertex Varying Main(Attribute in [[stage_in]],
                    constant Uniform& uniforms [[buffer(0)]])
#endif
{
#if SHADER_GLSL
    float4x4 world = float4x4(buf[0], buf[1], buf[2], buf[3]);
    float4x4 view = float4x4(buf[4], buf[5], buf[6], buf[7]);
    float4x4 projection = float4x4(buf[8], buf[9], buf[10], buf[11]);
#elif SHADER_METAL
    float4x4 world = uniforms.matrix[0];
    float4x4 view = uniforms.matrix[1];
    float4x4 projection = uniforms.matrix[2];
    float3 position = in.position;
    float2 uv = in.uv;
#endif
    float4 vpos = projection * (view * (world * float4(position, 1.0)));
#if SHADER_GLSL
    gl_Position = vpos;
    varyUV = uv;
#elif SHADER_METAL
    Varying out;
    out.varyPosition = vpos;
    out.varyUV = uv;
    return out;
#endif
}
#endif

#if SHADER_FRAGMENT
#if SHADER_GLSL
void main()
#elif SHADER_METAL
fragment float4 Main(Varying in [[stage_in]],
#if __METAL_USE_ARGUMENT__ == 0
                     TextureSampler textureSampler)
#else
                     constant TextureSampler& textureSampler [[buffer(0)]])
#endif
#endif
{
#if SHADER_GLSL
    gl_FragColor = texture2D(tex, varyUV);
#elif SHADER_METAL
    return textureSampler.tex.sample(textureSampler.sam, in.varyUV);
#endif
}
#endif)";
//-----------------------------------------------------------------------------
char const* ShaderWavefront::Illumination(int type)
{
    char const* graphic = xxGetInstanceName();
    if (graphic == nullptr)
        return "default";

    return IllumShaderCode;
}
//==============================================================================
