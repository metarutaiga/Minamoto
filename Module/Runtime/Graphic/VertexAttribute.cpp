//==============================================================================
// Minamoto : VertexAttribute Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <map>
#include <vector>
#include <xxGraphic.h>
#include "VertexAttribute.h"

//==============================================================================
static std::map<std::vector<int>, uint64_t> vertexAttributes;
//------------------------------------------------------------------------------
static uint64_t (*xxCreateVertexAttributeSystem)(uint64_t device, int count, int* attribute);
static void     (*xxDestroyVertexAttributeSystem)(uint64_t vertexAttribute);
//------------------------------------------------------------------------------
static uint64_t xxCreateVertexAttributeRuntime(uint64_t device, int count, int* attribute)
{
    std::vector<int> vector(attribute, attribute + count * 4);
    auto it = vertexAttributes.find(vector);
    if (it != vertexAttributes.end())
    {
        return (*it).second;
    }
    uint64_t output = xxCreateVertexAttributeSystem(device, count, attribute);
    if (output != 0)
    {
        vertexAttributes.insert(it, {vector, output});
    }
    return output;
}
//------------------------------------------------------------------------------
static void xxDestroyVertexAttributeRuntime(uint64_t vertexAttribute)
{
}
//==============================================================================
void VertexAttribute::Initialize()
{
    if (xxCreateVertexAttributeSystem)
        return;
    xxCreateVertexAttributeSystem = xxCreateVertexAttribute;
    xxDestroyVertexAttributeSystem = xxDestroyVertexAttribute;
    xxCreateVertexAttribute = xxCreateVertexAttributeRuntime;
    xxDestroyVertexAttribute = xxDestroyVertexAttributeRuntime;
}
//------------------------------------------------------------------------------
void VertexAttribute::Shutdown()
{
    if (xxCreateVertexAttributeSystem == nullptr)
        return;
    for (auto const& [vector, vertexAttribute] : vertexAttributes)
        xxDestroyVertexAttributeSystem(vertexAttribute);
    vertexAttributes.clear();
    xxCreateVertexAttribute = xxCreateVertexAttributeSystem;
    xxDestroyVertexAttribute = xxDestroyVertexAttributeSystem;
    xxCreateVertexAttributeSystem = nullptr;
    xxDestroyVertexAttributeSystem = nullptr;
}
//==============================================================================
