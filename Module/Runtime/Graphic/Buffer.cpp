//==============================================================================
// Minamoto : Buffer Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <deque>
#include <xxGraphicPlus/xxMesh.h>
#include "Buffer.h"

//==============================================================================
struct DestroyBuffer { size_t counter; uint64_t device; uint64_t buffer; };
static size_t Counter = 0;
static std::deque<DestroyBuffer> destroyBuffers;
//------------------------------------------------------------------------------
static void (*xxDestroyBufferSystem)(uint64_t device, uint64_t buffer);
//------------------------------------------------------------------------------
static void xxDestroyBufferRuntime(uint64_t device, uint64_t buffer)
{
    destroyBuffers.push_back({ Counter + 4, device, buffer });
}
//==============================================================================
void Buffer::Initialize()
{
    if (xxDestroyBufferSystem)
        return;
    xxDestroyBufferSystem = xxDestroyBuffer;
    xxDestroyBuffer = xxDestroyBufferRuntime;

    xxMesh::TransitionBufferCount(1);
}
//------------------------------------------------------------------------------
void Buffer::Update()
{
    Counter++;
    while (destroyBuffers.empty() == false)
    {
        auto& destroyBuffer = destroyBuffers.front();
        if (destroyBuffer.counter > Counter)
            return;
        xxDestroyBufferSystem(destroyBuffer.device, destroyBuffer.buffer);
        destroyBuffers.pop_front();
    }
}
//------------------------------------------------------------------------------
void Buffer::Shutdown()
{
    if (xxDestroyBufferSystem == nullptr)
        return;
    while (destroyBuffers.empty() == false)
    {
        auto& destroyBuffer = destroyBuffers.front();
        xxDestroyBufferSystem(destroyBuffer.device, destroyBuffer.buffer);
        destroyBuffers.pop_front();
    }
    xxDestroyBuffer = xxDestroyBufferSystem;
    xxDestroyBufferSystem = nullptr;
}
//==============================================================================
