//==============================================================================
// Minamoto : Pipeline Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <array>
#include <map>
#include <xxGraphic.h>
#include <internal/xxGraphicInternal.h>
#include "Pipeline.h"

//==============================================================================
static std::map<uint64_t, uint64_t>                 blendStates;
static uint64_t                                     depthStencilStates[1 << 4];
static uint64_t                                     rasterizerStates[1 << 2];
static std::map<std::array<uint64_t, 7>, uint64_t>  pipelines;
//------------------------------------------------------------------------------
static uint64_t (*xxCreateBlendStateSystem)(uint64_t device, char const* sourceColor, char const* operationColor, char const* destinationColor, char const* sourceAlpha, char const* operationAlpha, char const* destinationAlpha);
static uint64_t (*xxCreateDepthStencilStateSystem)(uint64_t device, char const* depthTest, bool depthWrite);
static uint64_t (*xxCreateRasterizerStateSystem)(uint64_t device, bool cull, bool scissor);
static uint64_t (*xxCreatePipelineSystem)(uint64_t device, uint64_t renderPass, uint64_t blendState, uint64_t depthStencilState, uint64_t rasterizerState, uint64_t vertexAttribute, uint64_t vertexShader, uint64_t fragmentShader);
static void     (*xxDestroyBlendStateSystem)(uint64_t blendState);
static void     (*xxDestroyDepthStencilStateSystem)(uint64_t depthStencilState);
static void     (*xxDestroyRasterizerStateSystem)(uint64_t rasterizerState);
static void     (*xxDestroyPipelineSystem)(uint64_t pipeline);
//------------------------------------------------------------------------------
static uint64_t xxCreateBlendStateRuntime(uint64_t device, char const* sourceColor, char const* operationColor, char const* destinationColor, char const* sourceAlpha, char const* operationAlpha, char const* destinationAlpha)
{
    auto xxBlendFactor = [](char const* name) { return xxTemplateBlendFactor<int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9>(name); };
    auto xxBlendOp = [](char const* name) { return xxTemplateBlendOp<int, 0, 1, 2, 3, 4>(name); };

    uint64_t hash = 0;
    hash |= xxBlendFactor(sourceColor)      << (0);
    hash |= xxBlendFactor(destinationColor) << (4);
    hash |= xxBlendOp(operationColor)       << (4 + 4);
    hash |= xxBlendFactor(sourceAlpha)      << (4 + 4 + 3);
    hash |= xxBlendFactor(destinationAlpha) << (4 + 4 + 3 + 4);
    hash |= xxBlendOp(operationAlpha)       << (4 + 4 + 3 + 4 + 4);

    auto it = blendStates.find(hash);
    if (it != blendStates.end())
    {
        return (*it).second;
    }
    uint64_t output = xxCreateBlendStateSystem(device, sourceColor, operationColor, destinationColor, sourceAlpha, operationAlpha, destinationAlpha);
    if (output != 0)
    {
        blendStates.insert(it, {hash, output});
    }
    return output;
}
//------------------------------------------------------------------------------
static uint64_t xxCreateDepthStencilStateRuntime(uint64_t device, char const* depthTest, bool depthWrite)
{
    auto xxCompareOp = [](char const* name) { return xxTemplateCompareOp<int, 0, 1, 2, 3, 4, 5, 6, 7>(name); };

    uint8_t hash = 0;
    hash |= xxCompareOp(depthTest)  << 0;
    hash |= depthWrite              << 3;

    uint64_t output = depthStencilStates[hash];
    if (output == 0)
    {
        output = depthStencilStates[hash] = xxCreateDepthStencilStateSystem(device, depthTest, depthWrite);
    }
    return output;
}
//------------------------------------------------------------------------------
static uint64_t xxCreateRasterizerStateRuntime(uint64_t device, bool cull, bool scissor)
{
    uint8_t hash = 0;
    hash |= cull    << 0;
    hash |= scissor << 1;

    uint64_t output = rasterizerStates[hash];
    if (output == 0)
    {
        output = rasterizerStates[hash] = xxCreateRasterizerStateSystem(device, cull, scissor);
    }
    return output;
}
//------------------------------------------------------------------------------
static uint64_t xxCreatePipelineRuntime(uint64_t device, uint64_t renderPass, uint64_t blendState, uint64_t depthStencilState, uint64_t rasterizerState, uint64_t vertexAttribute, uint64_t vertexShader, uint64_t fragmentShader)
{
    std::array<uint64_t, 7> hash;
    hash[0] = renderPass;
    hash[1] = blendState;
    hash[2] = depthStencilState;
    hash[3] = rasterizerState;
    hash[4] = vertexAttribute;
    hash[5] = vertexShader;
    hash[6] = fragmentShader;

    auto it = pipelines.find(hash);
    if (it != pipelines.end())
    {
        return (*it).second;
    }
    uint64_t output = xxCreatePipelineSystem(device, renderPass, blendState, depthStencilState, rasterizerState, vertexAttribute, vertexShader, fragmentShader);
    if (output != 0)
    {
        pipelines.insert(it, {hash, output});
    }
    return output;
}
//------------------------------------------------------------------------------
static void xxDestroyBlendStateRuntime(uint64_t blendState)
{
}
//------------------------------------------------------------------------------
static void xxDestroyDepthStencilStateRuntime(uint64_t depthStencilState)
{
}
//------------------------------------------------------------------------------
static void xxDestroyRasterizerStateRuntime(uint64_t rasterizerState)
{
}
//------------------------------------------------------------------------------
static void xxDestroyPipelineRuntime(uint64_t pipeline)
{
}
//==============================================================================
void Pipeline::Initialize()
{
    if (xxCreateBlendStateSystem)
        return;
    xxCreateBlendStateSystem = xxCreateBlendState;
    xxCreateDepthStencilStateSystem = xxCreateDepthStencilState;
    xxCreateRasterizerStateSystem = xxCreateRasterizerState;
    xxCreatePipelineSystem = xxCreatePipeline;
    xxDestroyBlendStateSystem = xxDestroyBlendState;
    xxDestroyDepthStencilStateSystem = xxDestroyDepthStencilState;
    xxDestroyRasterizerStateSystem = xxDestroyRasterizerState;
    xxDestroyPipelineSystem = xxDestroyPipeline;
    xxCreateBlendState = xxCreateBlendStateRuntime;
    xxCreateDepthStencilState = xxCreateDepthStencilStateRuntime;
    xxCreateRasterizerState = xxCreateRasterizerStateRuntime;
    xxCreatePipeline = xxCreatePipelineRuntime;
    xxDestroyBlendState = xxDestroyBlendStateRuntime;
    xxDestroyDepthStencilState = xxDestroyDepthStencilStateRuntime;
    xxDestroyRasterizerState = xxDestroyRasterizerStateRuntime;
    xxDestroyPipeline = xxDestroyPipelineRuntime;
}
//------------------------------------------------------------------------------
void Pipeline::Shutdown()
{
    if (xxCreateBlendStateSystem == nullptr)
        return;
    for (auto& [hash, blendState] : blendStates)
        xxDestroyBlendStateSystem(blendState);
    for (auto& depthStencilState : depthStencilStates)
    {
        if (depthStencilState)
        {
            xxDestroyDepthStencilStateSystem(depthStencilState);
            depthStencilState = 0;
        }
    }
    for (auto& rasterizerState : rasterizerStates)
    {
        if (rasterizerState)
        {
            xxDestroyRasterizerStateSystem(rasterizerState);
            rasterizerState = 0;
        }
    }
    for (auto& [hash, pipeline] : pipelines)
        xxDestroyPipelineSystem(pipeline);
    blendStates.clear();
    pipelines.clear();
    xxCreateBlendState = xxCreateBlendStateSystem;
    xxCreateDepthStencilState = xxCreateDepthStencilStateSystem;
    xxCreateRasterizerState = xxCreateRasterizerStateSystem;
    xxCreatePipeline = xxCreatePipelineSystem;
    xxDestroyBlendState = xxDestroyBlendStateSystem;
    xxDestroyDepthStencilState = xxDestroyDepthStencilStateSystem;
    xxDestroyRasterizerState = xxDestroyRasterizerStateSystem;
    xxDestroyPipeline = xxDestroyPipelineSystem;
    xxCreateBlendStateSystem = nullptr;
    xxCreateDepthStencilStateSystem = nullptr;
    xxCreateRasterizerStateSystem = nullptr;
    xxCreatePipelineSystem = nullptr;
    xxDestroyBlendStateSystem = nullptr;
    xxDestroyDepthStencilStateSystem = nullptr;
    xxDestroyRasterizerStateSystem = nullptr;
    xxDestroyPipelineSystem = nullptr;
}
//==============================================================================
