//==============================================================================
// Minamoto : PipelineDisassembly Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <string>
#include <vector>
#include <map>
#include <xxGraphic.h>
#include <utility/xxFile.h>
#include <Interface.h>
#include "ShaderDisassembly.h"

#if defined(__APPLE__)
#include <Metal/Metal.h>
#include <Metal/MTLBinaryArchive.h>
#endif

//==============================================================================
struct ShaderDisassemblyData
{
    uint64_t device = 0;
    uint64_t blendState = 0;
    uint64_t vertexAttribute = 0;
    uint64_t vertexShader = 0;
    uint64_t fragmentShader = 0;
    std::string vertexDisassembly;
    std::string fragmentDisassembly;
    bool disassembly = false;
};
static std::map<uint64_t, ShaderDisassemblyData> allShaderDisassembly;
//------------------------------------------------------------------------------
static uint64_t (*xxCreatePipelineSystem)(uint64_t device, uint64_t renderPass, uint64_t blendState, uint64_t depthStencilState, uint64_t rasterizerState, uint64_t vertexAttribute, uint64_t vertexShader, uint64_t fragmentShader);
//------------------------------------------------------------------------------
static uint64_t xxCreatePipelineRuntime(uint64_t device, uint64_t renderPass, uint64_t blendState, uint64_t depthStencilState, uint64_t rasterizerState, uint64_t vertexAttribute, uint64_t vertexShader, uint64_t fragmentShader)
{
    uint64_t output = xxCreatePipelineSystem(device, renderPass, blendState, depthStencilState, rasterizerState, vertexAttribute, vertexShader, fragmentShader);
    if (output)
    {
        allShaderDisassembly[output] = {device, blendState, vertexAttribute, vertexShader, fragmentShader};
    }
    return output;
}
//------------------------------------------------------------------------------
static void Disassemble(ShaderDisassemblyData& data)
{
    if (data.disassembly)
        return;
    data.disassembly = true;
#if defined(__APPLE__)
    if (@available(macOS 12, *))
    {
        NSError* error;
        id <MTLDevice> mtlDevice = (__bridge id <MTLDevice>)(void*)data.device;
        id <MTLBinaryArchive> mtlArchive = [mtlDevice newBinaryArchiveWithDescriptor:[MTLBinaryArchiveDescriptor new]
                                                                               error:&error];
        MTLRenderPipelineColorAttachmentDescriptor* mtlRenderPipelineColorAttachmentDescriptor = (__bridge MTLRenderPipelineColorAttachmentDescriptor*)(void*)data.blendState;
        MTLVertexDescriptor* mtlVertexDescriptor = (__bridge MTLVertexDescriptor*)(void*)data.vertexAttribute;
        id <MTLFunction> mtlVertexFunction = (__bridge id <MTLFunction>)(void*)data.vertexShader;
        id <MTLFunction> mtlFragmentFunction = (__bridge id <MTLFunction>)(void*)data.fragmentShader;
        MTLRenderPipelineDescriptor* mtlDesc = [MTLRenderPipelineDescriptor new];
        mtlDesc.vertexDescriptor = mtlVertexDescriptor;
        mtlDesc.vertexFunction = mtlVertexFunction;
        mtlDesc.fragmentFunction = mtlFragmentFunction;
        mtlDesc.colorAttachments[0] = mtlRenderPipelineColorAttachmentDescriptor;
        mtlDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
        [mtlArchive addRenderPipelineFunctionsWithDescriptor:mtlDesc
                                                       error:&error];
        if (error)
        {
            xxLog("Shader", "%s", error.localizedDescription.UTF8String);
        }
        [mtlArchive serializeToURL:[NSURL fileURLWithPath:@("/tmp/minamoto.pipeline.bin")]
                             error:&error];
        if (error)
        {
            xxLog("Shader", "%s", error.localizedDescription.UTF8String);
        }

        // Disassembly
        char command[1024];
        xxFile* file = xxFile::Load("/tmp/minamoto.pipeline.bin");
        if (file)
        {
            size_t size = file->Size();
            std::vector<uint32_t> archive(size / sizeof(uint32_t));
            file->Read(archive.data(), size);
            delete file;

            // Archive
            int found = 0;
            for (size_t i = 0; i < size / sizeof(uint32_t); ++i)
            {
                uint32_t magic = archive[i];
                if (magic != 0xFEEDFACF)
                    continue;

                if (found == 0)
                {
                    found++;

                    continue;
                }

                uint32_t size = archive[i + 0x90 / sizeof(uint32_t)];
                uint32_t offset = archive[i + 0x98 / sizeof(uint32_t)];
                size_t offset_prolog = i * sizeof(uint32_t) + offset;
                size_t offset_main = SIZE_T_MAX;
                for (size_t i = offset_prolog / sizeof(uint32_t); i < (offset_prolog + size) / sizeof(uint32_t); ++i)
                {
                    uint32_t code = archive[i];
                    if (code == 'HASH')
                        break;
                    if (code == 0x00080008)
                    {
                        for (size_t j = i; j < (offset_prolog + size) / sizeof(uint32_t); ++j)
                        {
                            code = archive[j];
                            if (code == 'HASH')
                                break;
                            if (code != 0x00080008)
                            {
                                offset_main = j * sizeof(uint32_t);
                                break;
                            }
                        }
                        break;
                    }
                }

                if (found == 1)
                {
                    found++;

                    snprintf(command, 1024, "/usr/bin/python3 %s/applegpu/disassemble.py /tmp/minamoto.pipeline.bin %zd > /tmp/minamoto.vertex.txt", xxGetDocumentPath(), offset_prolog);
                    system(command);
                    if (offset_main != SIZE_T_MAX)
                    {
                        snprintf(command, 1024, "echo -------------------------------------------- >> /tmp/minamoto.vertex.txt");
                        system(command);
                        snprintf(command, 1024, "/usr/bin/python3 %s/applegpu/disassemble.py /tmp/minamoto.pipeline.bin %zd >> /tmp/minamoto.vertex.txt", xxGetDocumentPath(), offset_main);
                        system(command);
                    }
                    file = xxFile::Load("/tmp/minamoto.vertex.txt");
                    if (file)
                    {
                        size_t size = file->Size();
                        data.vertexDisassembly.resize(size);
                        file->Read(data.vertexDisassembly.data(), size);
                        delete file;
                    }
                    remove("/tmp/minamoto.vertex.txt");
                    continue;
                }
                else if (found == 2)
                {
                    found++;

                    snprintf(command, 1024, "/usr/bin/python3 %s/applegpu/disassemble.py /tmp/minamoto.pipeline.bin %zd > /tmp/minamoto.fragment.txt", xxGetDocumentPath(), offset_prolog);
                    system(command);
                    if (offset_main != SIZE_T_MAX)
                    {
                        snprintf(command, 1024, "echo -------------------------------------------- >> /tmp/minamoto.fragment.txt");
                        system(command);
                        snprintf(command, 1024, "/usr/bin/python3 %s/applegpu/disassemble.py /tmp/minamoto.pipeline.bin %zd >> /tmp/minamoto.fragment.txt", xxGetDocumentPath(), offset_main);
                        system(command);
                    }
                    file = xxFile::Load("/tmp/minamoto.fragment.txt");
                    if (file)
                    {
                        size_t size = file->Size();
                        data.fragmentDisassembly.resize(size);
                        file->Read(data.fragmentDisassembly.data(), size);
                        delete file;
                    }
                    remove("/tmp/minamoto.fragment.txt");
                    break;
                }
            }
        }
        remove("/tmp/minamoto.pipeline.bin");
    }
#endif
}
//==============================================================================
void ShaderDisassembly::Initialize()
{
    if (xxCreatePipelineSystem)
        return;
    xxCreatePipelineSystem = xxCreatePipeline;
    xxCreatePipeline = xxCreatePipelineRuntime;
}
//------------------------------------------------------------------------------
void ShaderDisassembly::Shutdown()
{
    if (xxCreatePipelineSystem == nullptr)
        return;
    allShaderDisassembly.clear();
    xxCreatePipeline = xxCreatePipelineSystem;
    xxCreatePipelineSystem = nullptr;
}
//------------------------------------------------------------------------------
bool ShaderDisassembly::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    ImGui::SetNextWindowSize(ImVec2(1280.0f, 720.0f), ImGuiCond_Appearing);
    if (ImGui::Begin("Shader Disassembly", &show))
    {
        ImVec2 size = ImGui::GetWindowSize();
        size.x = size.x - ImGui::GetStyle().FramePadding.x * 8.0f;
        size.y = size.y - ImGui::GetCursorPosY() - (ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 6.0f);

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 256.0f);

        // Architecture
        static int archCurrent = 0;
        static const char* const archList[] =
        {
            "Apple G13G",
        };
        ImGui::SetNextItemWidth(128.0f);
        ImGui::Combo("Architecture", &archCurrent, archList, xxCountOf(archList));

        // Shader
        static int shaderCurrent = 0;
        ImGui::SetNextItemWidth(128.0f);
        ImGui::ListBox("Shader", &shaderCurrent, [](void*, int index) -> const char*
        {
            static char temp[64];
            auto it = allShaderDisassembly.begin();
            for (int i = 0; i < index; ++i)
                it++;
            auto& data = *(it);
            snprintf(temp, 64, "%016llX", data.first);
            return temp;
        }, nullptr, (int)allShaderDisassembly.size(), size.y / ImGui::GetTextLineHeightWithSpacing());

        auto it = allShaderDisassembly.begin();
        for (int i = 0; i < shaderCurrent; ++i)
            it++;
        auto& data = *(it);
        Disassemble(data.second);

        ImGui::NextColumn();

        // Shader
        static int type = 0;
        ImGui::RadioButton("Vertex Shader", &type, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Fragment Shader", &type, 1);

        if (type == 0)
        {
            ImGui::InputTextMultiline("", data.second.vertexDisassembly.data(), data.second.vertexDisassembly.size(), ImVec2(size.x - 128.0f, size.y), ImGuiInputTextFlags_ReadOnly);
        }
        else if (type == 1)
        {
            ImGui::InputTextMultiline("", data.second.fragmentDisassembly.data(), data.second.fragmentDisassembly.size(), ImVec2(size.x - 128.0f, size.y), ImGuiInputTextFlags_ReadOnly);
        }

        ImGui::Columns(1);
    }

    return false;
}
//==============================================================================
