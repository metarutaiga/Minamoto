//==============================================================================
// Minamoto : RenderPass Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include "RenderPass.h"

//==============================================================================
static uint64_t renderPasses[1 << 6];
//------------------------------------------------------------------------------
static uint64_t (*xxCreateRenderPassSystem)(uint64_t device, bool clearColor, bool clearDepth, bool clearStencil, bool storeColor, bool storeDepth, bool storeStencil);
static void     (*xxDestroyRenderPassSystem)(uint64_t renderPass);
//------------------------------------------------------------------------------
static uint64_t xxCreateRenderPassRuntime(uint64_t device, bool clearColor, bool clearDepth, bool clearStencil, bool storeColor, bool storeDepth, bool storeStencil)
{
    uint8_t hash = 0;
    hash |= clearColor      << 0;
    hash |= clearDepth      << 1;
    hash |= clearStencil    << 2;
    hash |= storeColor      << 3;
    hash |= storeDepth      << 4;
    hash |= storeStencil    << 5;

    uint64_t output = renderPasses[hash];
    if (output == 0)
    {
        output = renderPasses[hash] = xxCreateRenderPassSystem(device, clearColor, clearDepth, clearStencil, storeColor, storeDepth, storeStencil);
    }
    return output;
}
//------------------------------------------------------------------------------
static void xxDestroyRenderPassRuntime(uint64_t renderPass)
{
}
//==============================================================================
void RenderPass::Initialize()
{
    if (xxCreateRenderPassSystem)
        return;
    xxCreateRenderPassSystem = xxCreateRenderPass;
    xxDestroyRenderPassSystem = xxDestroyRenderPass;
    xxCreateRenderPass = xxCreateRenderPassRuntime;
    xxDestroyRenderPass = xxDestroyRenderPassRuntime;
}
//------------------------------------------------------------------------------
void RenderPass::Shutdown()
{
    if (xxCreateRenderPassSystem == nullptr)
        return;
    for (auto& renderPass : renderPasses)
    {
        if (renderPass)
        {
            xxDestroyRenderPassSystem(renderPass);
        }
        renderPass = 0;
    }
    xxCreateRenderPass = xxCreateRenderPassSystem;
    xxDestroyRenderPass = xxDestroyRenderPassSystem;
    xxCreateRenderPassSystem = nullptr;
    xxDestroyRenderPassSystem = nullptr;
}
//==============================================================================
