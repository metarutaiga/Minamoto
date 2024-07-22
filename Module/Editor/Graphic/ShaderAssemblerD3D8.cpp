//==============================================================================
// Minamoto : ShaderAssemblerD3D8 Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <Tools/WindowsHeader.h>
#include "ShaderAssemblerD3D8.h"

#ifdef _WIN32
#define strtok_r strtok_s
#endif

#include <dxsdk/d3d8types.h>
#include <dxsdk/d3d8caps.h>

#ifdef __clang__
#pragma clang diagnostic ignored "-Wextra-tokens"
#endif

#include <xxShaderAssembler/asm.h>
#include <xxShaderAssembler/disasm.h>

#include <xxShaderAssembler/microsoft/errlog.hpp>
#include <xxShaderAssembler/microsoft/valbase.hpp>
#include <xxShaderAssembler/microsoft/pshdrval.hpp>
#include <xxShaderAssembler/microsoft/vshdrval.hpp>
#include <xxShaderAssembler/microsoft/pshader.h>
#include <xxShaderAssembler/microsoft/vshader.h>

extern "C" void ShaderOutputDebugString(char const* string) {}

//==============================================================================
std::vector<uint32_t> ShaderAssemblerD3D8::Assemble(std::string const& shader, std::string& message)
{
    std::string lines = shader;
    std::vector<uint32_t> binary;

    char* lasts = nullptr;
    char const* line = strtok_r(lines.data(), "\n", &lasts);
    do
    {
        uint32_t tokens[8];
        size_t count = AssembleD3DSI(tokens, binary.empty() ? 0 : binary.front(), line);
        if (count > 8)
        {
            message += line;
            continue;
        }
        binary.insert(binary.end(), tokens, tokens + count);
    } while ((line = strtok_r(nullptr, "\n", &lasts)));
    binary.push_back(D3DSIO_END);

    // Validator
    if (binary.empty() == false && message.empty())
    {
        D3DCAPS8 caps = {};
        caps.VertexShaderVersion = D3DVS_VERSION(1, 1);
        caps.MaxVertexShaderConst = 96;
        caps.PixelShaderVersion = D3DPS_VERSION(1, 4);
        caps.MaxPixelShaderValue = 8.0f;

        CBaseShaderValidator* validator = nullptr;
        switch (binary[0])
        {
        case D3DVS_VERSION(1, 0):
        case D3DVS_VERSION(1, 1):
            validator = new CVShaderValidator((DWORD*)binary.data(), nullptr, &caps, SHADER_VALIDATOR_LOG_ERRORS);
            break;
        case D3DPS_VERSION(1, 0):
        case D3DPS_VERSION(1, 1):
        case D3DPS_VERSION(1, 2):
        case D3DPS_VERSION(1, 3):
            validator = new CPShaderValidator10((DWORD*)binary.data(), &caps, SHADER_VALIDATOR_LOG_ERRORS);
            break;
        case D3DPS_VERSION(1, 4):
            validator = new CPShaderValidator14((DWORD*)binary.data(), &caps, SHADER_VALIDATOR_LOG_ERRORS);
            break;
        default:
            break;
        }

        if (validator)
        {
            size_t pos = message.size();
            message.resize(pos + validator->GetRequiredLogBufferSize() - 1);
            validator->WriteLogToBuffer(message.data() + pos);
            delete validator;
        }
    }

    return binary;
}
//------------------------------------------------------------------------------
std::string ShaderAssemblerD3D8::Disassemble(std::vector<uint32_t> const& binary, std::string& message)
{
    size_t width = 1;
    for (size_t i = 0, count = 0; i < binary.size(); i += count)
    {
        uint32_t tokens[8] = {};
        count = TokenD3DSI(tokens, binary.data(), i, binary.size());
        if (count == 0)
        {
            count = CommentD3DSI(nullptr, 0, binary.data(), i, binary.size());
            if (count == 0)
                break;
            continue;
        }

        if (width < count)
            width = count;
    }

    std::string text;
    for (size_t i = 0, count = 0; i < binary.size(); i += count)
    {
        uint32_t tokens[8] = {};
        count = TokenD3DSI(tokens, binary.data(), i, binary.size());
        if (count == 0)
        {
            count = CommentD3DSI(nullptr, 0, binary.data(), i, binary.size());
            if (count == 0)
                break;
            continue;
        }

        char temp[256];
        snprintf(temp, 256, "%3zd: ", i);
        text += temp;
        DisassembleD3DSI(temp, 256, width, binary[0], tokens, count);
        text += temp;
        text += '\n';
    }

    if (binary.empty() == false)
    {
        char buffer[256];
        switch (binary[0])
        {
        case D3DVS_VERSION(1, 0):
        case D3DVS_VERSION(1, 1):
            for (size_t i = 1, count = 0; i < binary.size(); i += count)
            {
                uint32_t tokens[8] = {};
                count = TokenD3DSI(tokens, binary.data(), i, binary.size());
                if (count == 0)
                {
                    count = CommentD3DSI(nullptr, 0, binary.data(), i, binary.size());
                    if (count == 0)
                        break;
                    continue;
                }

                VertexShaderInstDisAsm(buffer, 256, (DWORD*)tokens, 0);
                message += buffer;
                message += '\n';
            }
            break;
        case D3DPS_VERSION(1, 0):
        case D3DPS_VERSION(1, 1):
        case D3DPS_VERSION(1, 2):
        case D3DPS_VERSION(1, 3):
        case D3DPS_VERSION(1, 4):
            for (size_t i = 1, count = 0; i < binary.size(); i += count)
            {
                uint32_t tokens[8] = {};
                count = TokenD3DSI(tokens, binary.data(), i, binary.size());
                if (count == 0)
                {
                    count = CommentD3DSI(nullptr, 0, binary.data(), i, binary.size());
                    if (count == 0)
                        break;
                    continue;
                }

                PixelShaderInstDisAsm(buffer, 256, (DWORD*)tokens, 0);
                message += buffer;
                message += '\n';
            }
            break;
        }
    }

    return text;
}
//==============================================================================
