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

#define DEBUG 0
#include <xxShaderAssembler/ati/chaplin/direct3d/hwvertexshader.c>
#include <xxShaderAssembler/ati/chaplin/direct3d/r200pixelshader.c>

//==============================================================================
static std::string* debugMessage = nullptr;
//------------------------------------------------------------------------------
VOID APIENTRY vDdHslDebugPrint(EDDHSLDEBUGLEVEL eDbgLevel, char* pDbgMsg, ...)
{
    if (debugMessage == nullptr)
        return;

    char temp[256];

    va_list arg;
    va_start(arg, pDbgMsg);
    vsnprintf(temp, 256, pDbgMsg, arg);
    va_end(arg);

    switch (eDbgLevel)
    {
    case E_GENERAL_ENTRY_EXIT:
#if 1
        return;
#else
        debugMessage->append("[ENTRY]");
        break;
#endif
    case E_ERROR_MESSAGE:
        debugMessage->append("[ERROR]");
        break;
    case E_PIXELSHADER_DATA:
        debugMessage->append("[PIXEL]");
        break;
    default:
        debugMessage->append("[?????]");
        break;
    }
    debugMessage->append(" ");
    debugMessage->append(temp);
    debugMessage->append("\n");
}
//------------------------------------------------------------------------------
std::vector<uint32_t> ShaderAssemblerR200::CompileChaplinPVS(std::vector<uint32_t> const& shader, std::string& message)
{
    debugMessage = &message;

    std::vector<uint32_t> code;
    if (shader.empty() == false && (shader.front() & 0xFFFF0000) == D3DVS_VERSION(0, 0))
    {
        if (message.empty())
        {
            for (uint32_t const& opcode : shader)
            {
                if (opcode & 0x80000000)
                    continue;
                if (opcode == D3DSIO_END)
                    break;

                DWORD pvsCode[4] = {};
                VS_HwAssemble(pvsCode, (DWORD*)&opcode);

                code.push_back(pvsCode[0]);
                code.push_back(pvsCode[1]);
                code.push_back(pvsCode[2]);
                code.push_back(pvsCode[3]);
            }
        }
    }

    debugMessage = nullptr;
    return code;
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
            PS_RESULT result = PS_RESULT_NONE;
            RAD2PIXELSHADER PS;
            D3DPSREGISTERS D3DPSRegs = {};

            Rad2InitPixelShader(&PS);
            PS.dwPixelShaderVersion = shader.front();

            for (uint32_t const& opcode : shader)
            {
                if (opcode & 0x80000000)
                    continue;
                if (opcode == D3DSIO_END)
                    break;
                result = Rad2CompilePixShaderInst(nullptr, (DWORD*)&opcode, &PS, result, &D3DPSRegs);
            }

            for (DWORD i = 0; i < PS_MAX_NUM_PHASES; ++i)
            {
                for (DWORD j = 0; j < PS_MAX_TEXSEQ; ++j)
                {
                    code.push_back(PS.TxModes[i][j].modes);
                    code.push_back(PS.TxModes[i][j].dwTexCoordSet);
                }
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
std::string ShaderAssemblerR200::DisassembleChaplinPVS(std::vector<uint32_t> const& code)
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
//------------------------------------------------------------------------------
std::string ShaderAssemblerR200::DisassembleChaplin(std::vector<uint32_t> const& code)
{
    std::string text;

    // TxMode
    for (size_t i = 0; i < code.size() && i < 12; i += 6)
    {
        char temp[256];
        snprintf(temp, 256, "%3zd: ", i);
        text += temp;

        for (size_t j = 0; j < 6; ++j)
        {
            snprintf(temp, 256, "%08X ", code[i + j]);
            text += temp;
        }

        text += '\n';
    }

    // psInstructions
    for (size_t i = 12; i < code.size(); i += 4)
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
