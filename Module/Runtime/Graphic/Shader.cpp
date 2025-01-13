//==============================================================================
// Minamoto : Shader Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <map>
#include <string_view>
#include "Shader.h"

//==============================================================================
static uint64_t defaultDevice;
static std::map<size_t, uint64_t> meshShaders;
static std::map<size_t, uint64_t> vertexShaders;
static std::map<size_t, uint64_t> fragmentShaders;
//------------------------------------------------------------------------------
static uint64_t (*xxCreateMeshShaderSystem)(uint64_t device, char const* shader);
static uint64_t (*xxCreateVertexShaderSystem)(uint64_t device, char const* shader, uint64_t vertexAttribute);
static uint64_t (*xxCreateFragmentShaderSystem)(uint64_t device, char const* shader);
static void     (*xxDestroyShaderSystem)(uint64_t device, uint64_t shader);
//------------------------------------------------------------------------------
static uint64_t xxCreateMeshShaderRuntime(uint64_t device, char const* shader)
{
    size_t hash = std::hash<std::string_view>()(shader);
    auto it = meshShaders.find(hash);
    if (it != meshShaders.end())
    {
        return (*it).second;
    }
    uint64_t output = xxCreateMeshShaderSystem(device, shader);
    if (output != 0)
    {
        defaultDevice = device;
        meshShaders.insert(it, {hash, output});
    }
    return output;
}
//------------------------------------------------------------------------------
static uint64_t xxCreateVertexShaderRuntime(uint64_t device, char const* shader, uint64_t vertexAttribute)
{
    size_t hash = std::hash<std::string_view>()(shader);
    auto it = vertexShaders.find(hash);
    if (it != vertexShaders.end())
    {
        return (*it).second;
    }
    uint64_t output = xxCreateVertexShaderSystem(device, shader, vertexAttribute);
    if (output != 0)
    {
        defaultDevice = device;
        vertexShaders.insert(it, {hash, output});
    }
    return output;
}
//------------------------------------------------------------------------------
static uint64_t xxCreateFragmentShaderRuntime(uint64_t device, char const* shader)
{
    size_t hash = std::hash<std::string_view>()(shader);
    auto it = fragmentShaders.find(hash);
    if (it != fragmentShaders.end())
    {
        return (*it).second;
    }
    uint64_t output = xxCreateFragmentShaderSystem(device, shader);
    if (output != 0)
    {
        defaultDevice = device;
        fragmentShaders.insert(it, {hash, output});
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
    if (xxCreateMeshShaderSystem)
        return;
    xxCreateMeshShaderSystem = xxCreateMeshShader;
    xxCreateVertexShaderSystem = xxCreateVertexShader;
    xxCreateFragmentShaderSystem = xxCreateFragmentShader;
    xxDestroyShaderSystem = xxDestroyShader;
    xxCreateMeshShader = xxCreateMeshShaderRuntime;
    xxCreateVertexShader = xxCreateVertexShaderRuntime;
    xxCreateFragmentShader = xxCreateFragmentShaderRuntime;
    xxDestroyShader = xxDestroyShaderRuntime;
}
//------------------------------------------------------------------------------
void Shader::Shutdown()
{
    if (xxCreateMeshShaderSystem == nullptr)
        return;
    for (auto const& [hash, shader] : meshShaders)
        xxDestroyShaderSystem(defaultDevice, shader);
    for (auto const& [hash, shader] : vertexShaders)
        xxDestroyShaderSystem(defaultDevice, shader);
    for (auto const& [hash, shader] : fragmentShaders)
        xxDestroyShaderSystem(defaultDevice, shader);
    defaultDevice = 0;
    meshShaders.clear();
    vertexShaders.clear();
    fragmentShaders.clear();
    xxCreateMeshShader = xxCreateMeshShaderSystem;
    xxCreateVertexShader = xxCreateVertexShaderSystem;
    xxCreateFragmentShader = xxCreateFragmentShaderSystem;
    xxDestroyShader = xxDestroyShaderSystem;
    xxCreateMeshShaderSystem = nullptr;
    xxCreateVertexShaderSystem = nullptr;
    xxCreateFragmentShaderSystem = nullptr;
    xxDestroyShaderSystem = nullptr;
}
//==============================================================================
