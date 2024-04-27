//==============================================================================
// Minamoto : Shader Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <string>
#include <unordered_map>
#include <xxGraphic.h>
#include "Shader.h"

//==============================================================================
static std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> vertexShaders;
static std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> fragmentShaders;
//------------------------------------------------------------------------------
static uint64_t (*xxCreateVertexShaderSystem)(uint64_t device, char const* shader, uint64_t vertexAttribute);
static uint64_t (*xxCreateFragmentShaderSystem)(uint64_t device, char const* shader);
static void     (*xxDestroyShaderSystem)(uint64_t device, uint64_t shader);
//------------------------------------------------------------------------------
static uint64_t xxCreateVertexShaderRuntime(uint64_t device, char const* shader, uint64_t vertexAttribute)
{
    auto it = vertexShaders.find(shader);
    if (it != vertexShaders.end())
    {
        return (*it).second.second;
    }
    uint64_t output = xxCreateVertexShaderSystem(device, shader, vertexAttribute);
    if (output != 0)
    {
        vertexShaders.insert(it, {shader, {device, output}});
    }
    return output;
}
//------------------------------------------------------------------------------
static uint64_t xxCreateFragmentShaderRuntime(uint64_t device, char const* shader)
{
    auto it = fragmentShaders.find(shader);
    if (it != fragmentShaders.end())
    {
        return (*it).second.second;
    }
    uint64_t output = xxCreateFragmentShaderSystem(device, shader);
    if (output != 0)
    {
        fragmentShaders.insert(it, {shader, {device, output}});
    }
    return output;
}
//------------------------------------------------------------------------------
static void xxDestroyShaderRuntime(uint64_t device, uint64_t shader)
{
}
//==============================================================================
void Shader::Initialize()
{
    if (xxCreateVertexShaderSystem)
        return;
    xxCreateVertexShaderSystem = xxCreateVertexShader;
    xxCreateFragmentShaderSystem = xxCreateFragmentShader;
    xxDestroyShaderSystem = xxDestroyShader;
    xxCreateVertexShader = xxCreateVertexShaderRuntime;
    xxCreateFragmentShader = xxCreateFragmentShaderRuntime;
    xxDestroyShader = xxDestroyShaderRuntime;
}
//------------------------------------------------------------------------------
void Shader::Shutdown()
{
    if (xxCreateVertexShaderSystem == nullptr)
        return;
    for (auto const& [shader, pair] : vertexShaders)
        xxDestroyShader(pair.first, pair.second);
    for (auto const& [shader, pair] : fragmentShaders)
        xxDestroyShader(pair.first, pair.second);
    vertexShaders.clear();
    fragmentShaders.clear();
    xxCreateVertexShader = xxCreateVertexShaderSystem;
    xxCreateFragmentShader = xxCreateFragmentShaderSystem;
    xxDestroyShader = xxDestroyShaderSystem;
    xxCreateVertexShaderSystem = nullptr;
    xxCreateFragmentShaderSystem = nullptr;
    xxDestroyShaderSystem = nullptr;
}
//==============================================================================
