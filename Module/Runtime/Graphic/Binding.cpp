//==============================================================================
// Minamoto : Binding Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphic/internal/xxGraphicInternal.h>
#include "Binding.h"

#if defined(xxMACOS) || defined(xxWINDOWS)
#include <xxGraphic/xxGraphicGlide.h>
#endif
#if defined(xxMACOS) || defined(xxIOS)
#include <xxGraphic/xxGraphicMetal2.h>
#endif

//==============================================================================
static struct { int x; int y; int width; int height; float minZ; float maxZ; } bindViewport;
static struct { int x; int y; int width; int height; } bindScissor;
static uint64_t bindPipeline;
static uint64_t bindVertexBuffers[16];
static uint64_t bindVertexTextures[16];
static uint64_t bindFragmentTextures[16];
static uint64_t bindVertexSamplers[16];
static uint64_t bindFragmentSamplers[16];
static uint64_t bindVertexConstantBuffer;
static uint64_t bindFragmentConstantBuffer;
//------------------------------------------------------------------------------
static void (*xxEndRenderPassSystem)(uint64_t commandEncoder, uint64_t framebuffer, uint64_t renderPass);
static void (*xxSetViewportSystem)(uint64_t commandEncoder, int x, int y, int width, int height, float minZ, float maxZ);
static void (*xxSetScissorSystem)(uint64_t commandEncoder, int x, int y, int width, int height);
static void (*xxSetPipelineSystem)(uint64_t commandEncoder, uint64_t pipeline);
static void (*xxSetVertexBuffersSystem)(uint64_t commandEncoder, int count, const uint64_t* buffers, uint64_t vertexAttribute);
static void (*xxSetVertexTexturesSystem)(uint64_t commandEncoder, int count, const uint64_t* textures);
static void (*xxSetFragmentTexturesSystem)(uint64_t commandEncoder, int count, const uint64_t* textures);
static void (*xxSetVertexSamplersSystem)(uint64_t commandEncoder, int count, const uint64_t* samplers);
static void (*xxSetFragmentSamplersSystem)(uint64_t commandEncoder, int count, const uint64_t* samplers);
static void (*xxSetVertexConstantBufferSystem)(uint64_t commandEncoder, uint64_t buffer, int size);
static void (*xxSetFragmentConstantBufferSystem)(uint64_t commandEncoder, uint64_t buffer, int size);
//------------------------------------------------------------------------------
static void xxEndRenderPassRuntime(uint64_t commandEncoder, uint64_t framebuffer, uint64_t renderPass)
{
    bindViewport = {};
    bindScissor = {};
    bindPipeline = 0;
    memset(bindVertexBuffers, 0, sizeof(bindVertexBuffers));
    memset(bindVertexTextures, 0, sizeof(bindVertexTextures));
    memset(bindFragmentTextures, 0, sizeof(bindFragmentTextures));
    memset(bindVertexSamplers, 0, sizeof(bindVertexSamplers));
    memset(bindFragmentSamplers, 0, sizeof(bindFragmentSamplers));
    bindVertexConstantBuffer = 0;
    bindFragmentConstantBuffer = 0;
    xxEndRenderPassSystem(commandEncoder, framebuffer, renderPass);
}
//------------------------------------------------------------------------------
static void xxSetViewportRuntime(uint64_t commandEncoder, int x, int y, int width, int height, float minZ, float maxZ)
{
    if (bindViewport.x == x && bindViewport.y == y && bindViewport.width == width && bindViewport.height == height && bindViewport.minZ == minZ && bindViewport.maxZ == maxZ)
        return;
    bindViewport = {x, y, width, height, minZ, maxZ};
    xxSetViewportSystem(commandEncoder, x, y, width, height, minZ, maxZ);
}
//------------------------------------------------------------------------------
static void xxSetScissorRuntime(uint64_t commandEncoder, int x, int y, int width, int height)
{
    if (bindScissor.x == x && bindScissor.y == y && bindScissor.width == width && bindScissor.height == height)
        return;
    bindScissor = {x, y, width, height};
    xxSetScissorSystem(commandEncoder, x, y, width, height);
}
//------------------------------------------------------------------------------
static void xxSetPipelineRuntime(uint64_t commandEncoder, uint64_t pipeline)
{
    if (bindPipeline == pipeline)
        return;
    bindPipeline = pipeline;
    xxSetPipelineSystem(commandEncoder, pipeline);
}
//------------------------------------------------------------------------------
static void xxSetVertexBuffersRuntime(uint64_t commandEncoder, int count, const uint64_t* buffers, uint64_t vertexAttribute)
{
    bool update = false;
    for (int i = 0; i < count; ++i)
    {
        if (bindVertexBuffers[i] != buffers[i])
        {
            bindVertexBuffers[i] = buffers[i];
            update = true;
        }
    }
    if (update == false)
        return;
    xxSetVertexBuffersSystem(commandEncoder, count, buffers, vertexAttribute);
}
//------------------------------------------------------------------------------
static void xxSetVertexTexturesRuntime(uint64_t commandEncoder, int count, const uint64_t* textures)
{
    bool update = false;
    for (int i = 0; i < count; ++i)
    {
        if (bindVertexTextures[i] != textures[i])
        {
            bindVertexTextures[i] = textures[i];
            update = true;
        }
    }
    if (update == false)
        return;
    xxSetVertexTexturesSystem(commandEncoder, count, textures);
}
//------------------------------------------------------------------------------
static void xxSetFragmentTexturesRuntime(uint64_t commandEncoder, int count, const uint64_t* textures)
{
    bool update = false;
    for (int i = 0; i < count; ++i)
    {
        if (bindFragmentTextures[i] != textures[i])
        {
            bindFragmentTextures[i] = textures[i];
            update = true;
        }
    }
    if (update == false)
        return;
    xxSetFragmentTexturesSystem(commandEncoder, count, textures);
}
//------------------------------------------------------------------------------
static void xxSetVertexSamplersRuntime(uint64_t commandEncoder, int count, const uint64_t* samplers)
{
    bool update = false;
    for (int i = 0; i < count; ++i)
    {
        if (bindVertexSamplers[i] != samplers[i])
        {
            bindVertexSamplers[i] = samplers[i];
            update = true;
        }
    }
    if (update == false)
        return;
    xxSetVertexSamplersSystem(commandEncoder, count, samplers);
}
//------------------------------------------------------------------------------
static void xxSetFragmentSamplersRuntime(uint64_t commandEncoder, int count, const uint64_t* samplers)
{
    bool update = false;
    for (int i = 0; i < count; ++i)
    {
        if (bindFragmentSamplers[i] != samplers[i])
        {
            bindFragmentSamplers[i] = samplers[i];
            update = true;
        }
    }
    if (update == false)
        return;
    xxSetFragmentSamplersSystem(commandEncoder, count, samplers);
}
//------------------------------------------------------------------------------
static void xxSetVertexConstantBufferRuntime(uint64_t commandEncoder, uint64_t buffer, int size)
{
    if (bindVertexConstantBuffer == buffer)
        return;
    bindVertexConstantBuffer = buffer;
    xxSetVertexConstantBufferSystem(commandEncoder, buffer, size);
}
//------------------------------------------------------------------------------
static void xxSetFragmentConstantBufferRuntime(uint64_t commandEncoder, uint64_t buffer, int size)
{
    if (bindFragmentConstantBuffer == buffer)
        return;
    bindFragmentConstantBuffer = buffer;
    xxSetFragmentConstantBufferSystem(commandEncoder, buffer, size);
}
//==============================================================================
void Binding::Initialize()
{
    if (xxEndRenderPassSystem)
        return;
    xxEndRenderPassSystem = xxEndRenderPass;
    xxSetViewportSystem = xxSetViewport;
    xxSetScissorSystem = xxSetScissor;
    xxSetPipelineSystem = xxSetPipeline;
    xxSetVertexBuffersSystem = xxSetVertexBuffers;
    xxSetVertexTexturesSystem = xxSetVertexTextures;
    xxSetFragmentTexturesSystem = xxSetFragmentTextures;
    xxSetVertexSamplersSystem = xxSetVertexSamplers;
    xxSetFragmentSamplersSystem = xxSetFragmentSamplers;
    xxSetVertexConstantBufferSystem = xxSetVertexConstantBuffer;
    xxSetFragmentConstantBufferSystem = xxSetFragmentConstantBuffer;
    xxEndRenderPass = xxEndRenderPassRuntime;
    xxSetViewport = xxSetViewportRuntime;
    xxSetScissor = xxSetScissorRuntime;
    xxSetPipeline = xxSetPipelineRuntime;
    xxSetVertexBuffers = xxSetVertexBuffersRuntime;
#if defined(xxMACOS) || defined(xxWINDOWS)
    if (xxGetInstanceName == xxGetInstanceNameGlide)
        return;
#endif
#if defined(xxMACOS) || defined(xxIOS)
    if (xxGetInstanceName == xxGetInstanceNameMetal2)
        return;
#endif
    xxSetVertexTextures = xxSetVertexTexturesRuntime;
    xxSetFragmentTextures = xxSetFragmentTexturesRuntime;
    xxSetVertexSamplers = xxSetVertexSamplersRuntime;
    xxSetFragmentSamplers = xxSetFragmentSamplersRuntime;
    xxSetVertexConstantBuffer = xxSetVertexConstantBufferRuntime;
    xxSetFragmentConstantBuffer = xxSetFragmentConstantBufferRuntime;
}
//------------------------------------------------------------------------------
void Binding::Shutdown()
{
    if (xxEndRenderPassSystem == nullptr)
        return;
    xxEndRenderPass = xxEndRenderPassSystem;
    xxSetViewport = xxSetViewportSystem;
    xxSetScissor = xxSetScissorSystem;
    xxSetPipeline = xxSetPipelineSystem;
    xxSetVertexBuffers = xxSetVertexBuffersSystem;
    xxSetVertexTextures = xxSetVertexTexturesSystem;
    xxSetFragmentTextures = xxSetFragmentTexturesSystem;
    xxSetVertexSamplers = xxSetVertexSamplersSystem;
    xxSetFragmentSamplers = xxSetFragmentSamplersSystem;
    xxSetVertexConstantBuffer = xxSetVertexConstantBufferSystem;
    xxSetFragmentConstantBuffer = xxSetFragmentConstantBufferSystem;
    xxEndRenderPassSystem = nullptr;
    xxSetViewportSystem = nullptr;
    xxSetScissorSystem = nullptr;
    xxSetPipelineSystem = nullptr;
    xxSetVertexBuffersSystem = nullptr;
    xxSetVertexTexturesSystem = nullptr;
    xxSetFragmentTexturesSystem = nullptr;
    xxSetVertexSamplersSystem = nullptr;
    xxSetFragmentSamplersSystem = nullptr;
    xxSetVertexConstantBufferSystem = nullptr;
    xxSetFragmentConstantBufferSystem = nullptr;
}
//==============================================================================
