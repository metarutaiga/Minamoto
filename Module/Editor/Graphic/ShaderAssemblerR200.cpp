//==============================================================================
// Minamoto : ShaderAssemblerR200 Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <assert.h>
#include <Tools/WindowsHeader.h>
#include "ShaderAssemblerR200.h"

#include <dxsdk/d3d8types.h>
#include <dxsdk/d3d8caps.h>
typedef float D3DVALUE;

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wwritable-strings"
#endif

#include <chaplin/atiddhsl.h>
#undef HSLDPF
#undef HSLASSERT
#define HSLDPF( level, ... ) ShaderAssemblerR200::DebugPrintf(true, __VA_ARGS__)
#define HSLASSERT( exp ) ( exp ? (void)0 : ShaderAssemblerR200::DebugPrintf(true, #exp) )
#include <chaplin/r200pixelshader.c>

//==============================================================================
static std::string* debugMessage = nullptr;
//------------------------------------------------------------------------------
void ShaderAssemblerR200::DebugPrintf(bool breakline, char const* format, ...)
{
    if (debugMessage == nullptr)
        return;

    char temp[256];

    va_list arg;
    va_start(arg, format);
    vsnprintf(temp, 256, format, arg);
    va_end(arg);

    debugMessage->append(temp);
    if (breakline)
    {
        debugMessage->append("\n");
    }
}
//------------------------------------------------------------------------------
std::vector<uint32_t> ShaderAssemblerR200::CompileChaplin(std::vector<uint32_t> const& shader, std::string& message)
{
    debugMessage = &message;

    std::vector<uint32_t> code;
    if (shader.empty() == false && (shader.front() & 0xFFFF0000) == D3DPS_VERSION(0, 0))
    {
        if (message.empty())
        {
            RAD2PIXELSHADER PS = {};
            D3DPSREGISTERS D3DPSRegs = {};

            for (size_t i = 1; i < shader.size(); ++i)
            {
                uint32_t const& opcode = shader[i];
                if (opcode & 0x80000000)
                    continue;
                if (opcode == D3DSIO_END)
                    break;
                Rad2CompilePixShaderInst(nullptr, (DWORD*)&opcode, &PS, PS_OK, &D3DPSRegs);
            }

            for (DWORD i = 0; i < PS_MAX_NUM_PHASES; ++i)
            {
                for (DWORD j = 0; j < PS.dwInstrCount[i]; ++j)
                {
                    code.push_back(PS.psInstructions[i][j].C0);
                    code.push_back(PS.psInstructions[i][j].C1);
                    code.push_back(PS.psInstructions[i][j].A0);
                    code.push_back(PS.psInstructions[i][j].A1);
                }
            }
        }
    }

    debugMessage = nullptr;
    return code;
}
//------------------------------------------------------------------------------
std::string ShaderAssemblerR200::DisassembleChaplin(std::vector<uint32_t> const& code)
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
