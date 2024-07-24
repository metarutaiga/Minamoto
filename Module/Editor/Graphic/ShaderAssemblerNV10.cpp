//==============================================================================
// Minamoto : ShaderAssemblerNV10 Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <assert.h>
#include <Tools/WindowsHeader.h>
#include "ShaderAssemblerNV10.h"

#include <xxShaderAssembler/disasm.h>
#include <xxShaderAssembler/nvidia/celsius/cheops.h>

//------------------------------------------------------------------------------
static std::string* debugMessage = nullptr;
//------------------------------------------------------------------------------
int ShaderAssemblerNV10::DebugPrintf(char const* format, ...)
{
    if (debugMessage == nullptr)
        return 0;

    char temp[256];

    va_list arg;
    va_start(arg, format);
    vsnprintf(temp, 256, format, arg);
    va_end(arg);

    debugMessage->append(temp);

    return 0;
}
//------------------------------------------------------------------------------
std::vector<uint32_t> ShaderAssemblerNV10::CompileCheops(std::vector<uint32_t> const& binary, std::string& message)
{
    PrintfCheops = DebugPrintf;
    debugMessage = &message;

    std::vector<uint32_t> code;
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
        uint64_t cheops[4];
        size_t size = CompileCheopsFromD3DSI(cheops, tokens);
        code.insert(code.end(), (uint32_t*)cheops, (uint32_t*)cheops + size * 2);
    }

    debugMessage = nullptr;
    return code;
}
//------------------------------------------------------------------------------
std::vector<uint32_t> ShaderAssemblerNV10::CompileCelsius(std::vector<uint32_t> const& binary, std::string& message)
{
    std::vector<uint32_t> code;
    return code;
}
//------------------------------------------------------------------------------
std::string ShaderAssemblerNV10::DisassembleCheops(std::vector<uint32_t> const& code, std::string& message)
{
    PrintfCheops = DebugPrintf;
    debugMessage = &message;

    std::string text;
    for (size_t i = 0; i < code.size(); i += 2)
    {
        char temp[256];
        snprintf(temp, 256, "%3zd: ", i);
        text += temp;

        for (size_t j = 0; j < 2; ++j)
        {
            snprintf(temp, 256, "%08X ", code[i + j]);
            text += temp;
        }

        uint64_t cheops;
        memcpy(&cheops, code.data() + i, sizeof(uint64_t));
        DisasembleCheops(temp, 256, cheops);
        text += temp;

        text += '\n';
    }

    debugMessage = nullptr;
    return text;
}
//------------------------------------------------------------------------------
std::string ShaderAssemblerNV10::DisassembleCelsius(std::vector<uint32_t> const& code, std::string& message)
{
    std::string text;
    for (size_t i = 0; i < code.size(); i += 4)
    {
        char temp[256];
        snprintf(temp, 256, "%3zd: ", i);
        text += temp;

        for (size_t j = 0; j < 4; ++j)
        {
            snprintf(temp, 256, "%08X ", code[i + j]);
            text += temp;
        }

        text += '\n';
    }
    return text;
}
//==============================================================================
