//==============================================================================
// Minamoto : ShaderAssemblerNV20 Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <assert.h>
#include "ShaderAssemblerNV20.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
typedef float FLOAT;
typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int16_t SHORT;
typedef int32_t INT;
typedef int32_t LONG;
typedef uint16_t USHORT;
typedef uint32_t UINT;
typedef uint64_t UINT64;
typedef int32_t HRESULT;
typedef uint64_t LARGE_INTEGER;
typedef void* HANDLE;
typedef void* HWND;
typedef struct { char x[16]; } GUID;
#define TRUE true
#define FALSE false
#define DEFINE_GUID(...)
#endif

#include <dxsdk/d3d8types.h>
#include <dxsdk/d3d8caps.h>
typedef float D3DVALUE;

#define _DEBUG
#define DEBUG
#define dbgLevel 0xFFFFFFFF
#define DPF(...) ShaderAssemblerNV20::DebugPrintf(true, __VA_ARGS__)
#define DPF_PLAIN(...) ShaderAssemblerNV20::DebugPrintf(false, __VA_ARGS__)
#define DPF_LEVEL(level, ...) ShaderAssemblerNV20::DebugPrintf(true, __VA_ARGS__)
#define DPF_LEVEL_PLAIN(level, ...) ShaderAssemblerNV20::DebugPrintf(false, __VA_ARGS__)
#define AllocIPM malloc
#define FreeIPM free
#define NVARCH 0x20
#define nvAssert assert
#include <kelvin/nvPShad.cpp>
#include <kelvin/nvKelvinProgram.cpp>
#include <kelvin/vpcompilekelvin.c>
#include <kelvin/vpoptimize.c>
static GLOBALDATA DriverData;
GLOBALDATA* pDriverData = &DriverData;

//==============================================================================
struct RegisterCombinerInput
{
    struct
    {
        uint8_t reg:4;
        uint8_t alpha:1;
        uint8_t mapping:3;
    } d;
    struct
    {
        uint8_t reg:4;
        uint8_t alpha:1;
        uint8_t mapping:3;
    } c;
    struct
    {
        uint8_t reg:4;
        uint8_t alpha:1;
        uint8_t mapping:3;
    } b;
    struct
    {
        uint8_t reg:4;
        uint8_t alpha:1;
        uint8_t mapping:3;
    } a;
};
//------------------------------------------------------------------------------
union RegisterCombinerOutput
{
    struct
    {
        uint32_t reg:4;
        uint32_t __:8;
        uint32_t dp:1;
    } cd;
    struct
    {
        uint32_t _:4;
        uint32_t reg:4;
        uint32_t __:5;
        uint32_t dp:1;
    } ab;
    struct
    {
        uint32_t _:8;
        uint32_t reg:4;
        uint32_t __:2;
        uint32_t mux:1;
    } sum;
    struct
    {
        uint32_t _:15;
        uint32_t shift:3;
    };
};
//------------------------------------------------------------------------------
static std::string* debugMessage = nullptr;
//------------------------------------------------------------------------------
void ShaderAssemblerNV20::DebugPrintf(bool breakline, char const* format, ...)
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
std::vector<uint32_t> ShaderAssemblerNV20::CompileCheops(std::vector<uint32_t> const& shader, std::string& message)
{
    debugMessage = &message;

    std::vector<uint32_t> code;
    if (shader.empty() == false && (shader.front() & 0xFFFF0000) == D3DVS_VERSION(0, 0))
    {
        KELVIN_PROGRAM program = {};
        ParsedProgram parsed = {};
        nvKelvinParseVertexShaderCode(&program, &parsed, (DWORD*)shader.data(), (DWORD)shader.size());

        parsed.firstInstruction = program.code;
        parsed.liveOnEntry = FALSE;
        parsed.IsStateProgram = FALSE;
        vp_Optimize(&parsed, program.dwNumInstructions, 0);

        VtxProgCompileKelvin env = {};
        env.caller_id = CALLER_ID_D3D;
        env.malloc = [](void*, size_t size) { return malloc(size); };
        env.free = [](void*, void* pointer) { return free(pointer); };

        VertexProgramOutput output = {};
        vp_CompileKelvin(&env, &parsed, program.dwNumInstructions, &output);
        code.assign((uint32_t*)output.residentProgram, (uint32_t*)output.residentProgram + output.residentSize / sizeof(uint32_t));
        free(output.residentProgram);
    }

    debugMessage = nullptr;
    return code;
}
//------------------------------------------------------------------------------
std::vector<uint32_t> ShaderAssemblerNV20::CompileKelvin(std::vector<uint32_t> const& shader, std::string& message)
{
    debugMessage = &message;

    std::vector<uint32_t> code;
    if (shader.empty() == false && (shader.front() & 0xFFFF0000) == D3DPS_VERSION(0, 0))
    {
        if (message.empty())
        {
            DriverData.nvD3DPerfData.dwNVClasses = NVCLASS_FAMILY_KELVIN;

            struct CPixelShaderPublic : public CPixelShader
            {
                using CPixelShader::m_dwStage;
                using CPixelShader::m_cw;
                using CPixelShader::m_pixelShaderConsts;
                using CPixelShader::m_dwPixelShaderConstFlags;
                using CPixelShader::GetShaderProgram;
            };
            CPixelShaderPublic pixelShader;
            pixelShader.create(nullptr, 0, (DWORD)shader.size(), (DWORD*)shader.data());

            DWORD shaderPrograms[4] = {};
            for (DWORD i = 1; i < 4; ++i)
            {
                shaderPrograms[i] |= DRF_NUM(097, _SET_SHADER_STAGE_PROGRAM, _STAGE0, pixelShader.GetShaderProgram(nullptr, 0, i));
                shaderPrograms[i] |= DRF_NUM(097, _SET_SHADER_STAGE_PROGRAM, _STAGE1, pixelShader.GetShaderProgram(nullptr, 1, i));
                shaderPrograms[i] |= DRF_NUM(097, _SET_SHADER_STAGE_PROGRAM, _STAGE2, pixelShader.GetShaderProgram(nullptr, 2, i));
                shaderPrograms[i] |= DRF_NUM(097, _SET_SHADER_STAGE_PROGRAM, _STAGE3, pixelShader.GetShaderProgram(nullptr, 3, i));
            }

            code.push_back(pixelShader.m_dwStage);
            code.push_back(shaderPrograms[1]);
            code.push_back(shaderPrograms[2]);
            code.push_back(shaderPrograms[3]);

            DebugPrintf(true, "**************** %s ****************", "Texture Shaders");

            DebugPrintf(true, "%1s|%32s|%32s|%32s|", "", "2D", "3D", "CUBE");

            DebugPrintf(false, "%1s|", "-");
            DebugPrintf(false, "%32s|", "--------------------------------");
            DebugPrintf(false, "%32s|", "--------------------------------");
            DebugPrintf(true,  "%32s|", "--------------------------------");

            for (size_t i = 0; i < 4; ++i)
            {
                static char const* const stage[32] =
                {
                    "PROGRAM_NONE",
                    "2D_PROJECTIVE",
                    "3D_PROJECTIVE",
                    "CUBE_MAP",
                    "PASS_THROUGH",
                    "CLIP_PLANE",
                    "BUMPENVMAP",
                    "BUMPENVMAP_LUMINANCE",
                    "BRDF",
                    "DOT_ST",
                    "DOT_ZW",
                    "DOT_REFLECT_DIFFUSE",
                    "DOT_REFLECT_SPECULAR",
                    "DOT_STR_3D",
                    "DOT_STR_CUBE",
                    "DEPENDENT_AR",
                    "DEPENDENT_GB",
                    "DOT_PRODUCT",
                    "DOT_REFLECT_SPECULAR_CONST",
                };

                DebugPrintf(false, "%zd|", i);
                for (size_t j = 1; j < 4; ++j)
                {
                    DebugPrintf(j == 3, "%32s|", stage[(shaderPrograms[j] >> (i * 5)) & 0x1F]);
                }
            }

            DebugPrintf(true, "**************** %s ****************", "Register Combiners");

            DebugPrintf(false, "%18s|%18s|%18s|%18s|", "A", "B", "C", "D");
            DebugPrintf(false, "%1s|", "");
            DebugPrintf(true,  "%18s|%18s|%18s|%18s|", "A", "B", "C", "D");

            for (size_t i = 0; i < pixelShader.m_dwStage; ++i)
            {
                uint32_t colorICW32 = pixelShader.m_cw[i][PSHAD_COLOR][PSHAD_ICW];
                uint32_t colorOCW32 = pixelShader.m_cw[i][PSHAD_COLOR][PSHAD_OCW];
                uint32_t alphaICW32 = pixelShader.m_cw[i][PSHAD_ALPHA][PSHAD_ICW];
                uint32_t alphaOCW32 = pixelShader.m_cw[i][PSHAD_ALPHA][PSHAD_OCW];

                code.push_back(colorICW32);
                code.push_back(colorOCW32);
                code.push_back(alphaICW32);
                code.push_back(alphaOCW32);

                RegisterCombinerInput colorICW = (RegisterCombinerInput&)colorICW32;
                RegisterCombinerOutput colorOCW = (RegisterCombinerOutput&)colorOCW32;
                RegisterCombinerInput alphaICW = (RegisterCombinerInput&)alphaICW32;
                RegisterCombinerOutput alphaOCW = (RegisterCombinerOutput&)alphaOCW32;

                DebugPrintf(false, "%18s|", "------------------");
                DebugPrintf(false, "%18s|", "------------------");
                DebugPrintf(false, "%18s|", "------------------");
                DebugPrintf(false, "%18s|", "------------------");
                DebugPrintf(false, "%1s|", "-");
                DebugPrintf(false, "%18s|", "------------------");
                DebugPrintf(false, "%18s|", "------------------");
                DebugPrintf(false, "%18s|", "------------------");
                DebugPrintf(true,  "%18s|", "------------------");

                static char const* const reg[16] =
                {
                    "ZERO",
                    "CONSTANT0",
                    "CONSTANT1",
                    "FOG",
                    "COLOR0",
                    "COLOR1",
                    "TEXTURE4",
                    "TEXTURE5",
                    "TEXTURE0",
                    "TEXTURE1",
                    "TEXTURE2",
                    "TEXTURE3",
                    "SPARE0",
                    "SPARE1",
                    "TEXTURE6",
                    "TEXTURE7",
                };

                DebugPrintf(false, "%18s|", colorICW.a.alpha ? "ALPHA" : "RGB");
                DebugPrintf(false, "%18s|", colorICW.b.alpha ? "ALPHA" : "RGB");
                DebugPrintf(false, "%18s|", colorICW.c.alpha ? "ALPHA" : "RGB");
                DebugPrintf(false, "%18s|", colorICW.d.alpha ? "ALPHA" : "RGB");
                DebugPrintf(false, "%1zd|", i);
                DebugPrintf(false, "%18s|", alphaICW.a.alpha ? "ALPHA" : "RGB");
                DebugPrintf(false, "%18s|", alphaICW.b.alpha ? "ALPHA" : "RGB");
                DebugPrintf(false, "%18s|", alphaICW.c.alpha ? "ALPHA" : "RGB");
                DebugPrintf(true,  "%18s|", alphaICW.d.alpha ? "ALPHA" : "RGB");

                DebugPrintf(false, "%18s|", reg[colorICW.a.reg]);
                DebugPrintf(false, "%18s|", reg[colorICW.b.reg]);
                DebugPrintf(false, "%18s|", reg[colorICW.c.reg]);
                DebugPrintf(false, "%18s|", reg[colorICW.d.reg]);
                DebugPrintf(false, "%1s|", "");
                DebugPrintf(false, "%18s|", reg[alphaICW.a.reg]);
                DebugPrintf(false, "%18s|", reg[alphaICW.b.reg]);
                DebugPrintf(false, "%18s|", reg[alphaICW.c.reg]);
                DebugPrintf(true,  "%18s|", reg[alphaICW.d.reg]);

                static char const* const mapping[8] =
                {
                    "UNSIGNED_IDENTITY",
                    "UNSIGNED_INVERT",
                    "EXPAND_NORMAL",
                    "EXPAND_NEGATE",
                    "HALFBIAS_NORMAL",
                    "HALFBIAS_NEGATE",
                    "SIGNED_IDENTITY",
                    "SIGNED_NEGATE",
                };

                DebugPrintf(false, "%18s|", mapping[colorICW.a.mapping]);
                DebugPrintf(false, "%18s|", mapping[colorICW.b.mapping]);
                DebugPrintf(false, "%18s|", mapping[colorICW.c.mapping]);
                DebugPrintf(false, "%18s|", mapping[colorICW.d.mapping]);
                DebugPrintf(false, "%1s|", "");
                DebugPrintf(false, "%18s|", mapping[alphaICW.a.mapping]);
                DebugPrintf(false, "%18s|", mapping[alphaICW.b.mapping]);
                DebugPrintf(false, "%18s|", mapping[alphaICW.c.mapping]);
                DebugPrintf(true,  "%18s|", mapping[alphaICW.d.mapping]);

                char temp[256];
                auto op = [&temp](char const* a, char const* op, char const* b, int c, int d) -> char const*
                {
                    char const* format = "";
                    switch (c)
                    {
                    case 0: format = "%s %s %s";                break;
                    case 1: format = "(%s %s %s - 0.5)";        break;
                    case 2: format = "(%s %s %s) * 2.0";        break;
                    case 3: format = "(%s %s %s - 0.5) * 2.0";  break;
                    case 4: format = "(%s %s %s) * 4.0";        break;
                    case 5: format = "(%s %s %s - 0.5) * 4.0";  break;
                    case 6: format = "(%s %s %s) / 2.0";        break;
                    case 7: format = "(%s %s %s - 0.5) / 2.0";  break;
                    }
                    snprintf(temp, 256, format, a, op, b);
                    if (d != NV_REG_ZERO)
                    {
                        strcat(temp, " = ");
                        strcat(temp, reg[d]);
                    }
                    return temp;
                };

                DebugPrintf(false, "%37s|", op("A", colorOCW.ab.dp ? "DOT" : "MUL", "B", colorOCW.shift ,colorOCW.ab.reg));
                DebugPrintf(false, "%37s|", op("C", colorOCW.cd.dp ? "DOT" : "MUL", "D", colorOCW.shift ,colorOCW.cd.reg));
                DebugPrintf(false, "%1s|", "");
                DebugPrintf(false, "%37s|", op("A", alphaOCW.ab.dp ? "DOT" : "MUL", "B", alphaOCW.shift ,alphaOCW.ab.reg));
                DebugPrintf(true,  "%37s|", op("C", alphaOCW.cd.dp ? "DOT" : "MUL", "D", alphaOCW.shift ,alphaOCW.cd.reg));

                DebugPrintf(false, "%75s|", op("AB", colorOCW.sum.mux ? "MUX" : "SUM", "CD", colorOCW.shift, colorOCW.sum.reg));
                DebugPrintf(false, "%1s|", "");
                DebugPrintf(true,  "%75s|", op("AB", alphaOCW.sum.mux ? "MUX" : "SUM", "CD", alphaOCW.shift, alphaOCW.sum.reg));
            }

            size_t max = 0;
            for (size_t i = 0; i < PSHAD_MAX_CONSTANTS; ++i)
            {
                if (pixelShader.m_dwPixelShaderConstFlags[i])
                    max = i + 1;
            }

            for (size_t i = 0; i < max; ++i)
            {
                uint32_t r = pixelShader.m_dwPixelShaderConstFlags[i] ? (uint32_t&)pixelShader.m_pixelShaderConsts[i].r : 0;
                uint32_t g = pixelShader.m_dwPixelShaderConstFlags[i] ? (uint32_t&)pixelShader.m_pixelShaderConsts[i].g : 0;
                uint32_t b = pixelShader.m_dwPixelShaderConstFlags[i] ? (uint32_t&)pixelShader.m_pixelShaderConsts[i].b : 0;
                uint32_t a = pixelShader.m_dwPixelShaderConstFlags[i] ? (uint32_t&)pixelShader.m_pixelShaderConsts[i].a : 0;
                code.push_back(b);
                code.push_back(g);
                code.push_back(r);
                code.push_back(a);
            }
        }
    }

    debugMessage = nullptr;
    return code;
}
//------------------------------------------------------------------------------
std::string ShaderAssemblerNV20::DisassembleCheops(std::vector<uint32_t> const& code)
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
std::string ShaderAssemblerNV20::DisassembleKelvin(std::vector<uint32_t> const& code)
{
    char temp[256];
    std::string text;

    // ???
    if (code.size() < 4)
        return text;

    // Stage
    size_t dwStage = code.front();
    if (code.size() < (4 + dwStage * 4))
        return text;

    // Stage
    for (size_t i = 0; i < 4; i += 4)
    {
        snprintf(temp, 256, "%3zd: ", i);
        text += temp;

        for (size_t j = 0; j < 4; ++j)
        {
            snprintf(temp, 256, "%08X ", code[i + j]);
            text += temp;
        }

        text += '\n';
    }

    // Register Combiner
    for (size_t i = 4; i < (4 + dwStage * 4); i += 4)
    {
        snprintf(temp, 256, "%3zd: ", i);
        text += temp;

        for (size_t j = 0; j < 4; ++j)
        {
            snprintf(temp, 256, "%08X ", code[i + j]);
            text += temp;
        }

        auto reg = [&temp](int index, char const* swizzle, int mapping) -> char const*
        {
            if (index == NV_REG_ZERO)
            {
                switch (mapping)
                {
                case NV_MAPPING_UNSIGNED_IDENTITY:  return "0";
                case NV_MAPPING_UNSIGNED_INVERT:    return "1";
                case NV_MAPPING_EXPAND_NORMAL:      return "-1";
                case NV_MAPPING_EXPAND_NEGATE:      return "1";
                case NV_MAPPING_HALF_BIAS_NORMAL:   return "-0.5";
                case NV_MAPPING_HALF_BIAS_NEGATE:   return "0.5";
                case NV_MAPPING_SIGNED_IDENTITY:    return "0";
                case NV_MAPPING_SIGNED_NEGATE:      return "0";
                }
            }
            char const* prefix = "";
            char const* suffix = "";
            switch (mapping)
            {
            case NV_MAPPING_UNSIGNED_IDENTITY:                                      break;
            case NV_MAPPING_UNSIGNED_INVERT:    prefix = "1-";                      break;
            case NV_MAPPING_EXPAND_NORMAL:                      suffix = "_bx2";    break;
            case NV_MAPPING_EXPAND_NEGATE:      prefix = "-";   suffix = "_bx2";    break;
            case NV_MAPPING_HALF_BIAS_NORMAL:                   suffix = "_bias";   break;
            case NV_MAPPING_HALF_BIAS_NEGATE:   prefix = "-";   suffix = "_bias";   break;
            case NV_MAPPING_SIGNED_IDENTITY:                                        break;
            case NV_MAPPING_SIGNED_NEGATE:      prefix = "-";                       break;
            }
            snprintf(temp, 256, "%sr%d%s", prefix, index, suffix);
            if (swizzle)
            {
                strcat(temp, ".");
                strcat(temp, swizzle);
            }
            return temp;
        };

        int instruction = 0;
        auto next = [&]()
        {
            if (instruction)
            {
                text += '\n';
                text.append(5 + 9 * 4, ' ');
                text += '+';
            }
            instruction++;
        };

        static char const* const swizzleText[3] = { nullptr, "rgb", "a" };
        static char const* const shiftText[8] =
        {
            "",
            "_bias",
            "_x2",
            "_bx2",
            "_x4",
            "_bx4",
            "_d2",
            "_d4",
        };

        RegisterCombinerInput colorICW = (RegisterCombinerInput&)code[i + 0];
        RegisterCombinerOutput colorOCW = (RegisterCombinerOutput&)code[i + 1];
        RegisterCombinerInput alphaICW = (RegisterCombinerInput&)code[i + 2];
        RegisterCombinerOutput alphaOCW = (RegisterCombinerOutput&)code[i + 3];

        // MMA
        if (true)
        {
            auto mma = [](char const*& opcode, int& args, int o, int m,
                          int a, int b, int c, int d,
                          int x, int y, int z, int w)
            {
                if (o == NV_REG_ZERO)
                    return;
                if (x == NV_MAPPING_SIGNED_IDENTITY)
                    x = NV_MAPPING_UNSIGNED_IDENTITY;
                if (y == NV_MAPPING_SIGNED_IDENTITY)
                    y = NV_MAPPING_UNSIGNED_IDENTITY;
                if (z == NV_MAPPING_SIGNED_IDENTITY)
                    z = NV_MAPPING_UNSIGNED_IDENTITY;
                if (w == NV_MAPPING_SIGNED_IDENTITY)
                    w = NV_MAPPING_UNSIGNED_IDENTITY;
                if (b == NV_REG_ZERO && y == NV_MAPPING_UNSIGNED_INVERT && d == NV_REG_ZERO && w == NV_MAPPING_UNSIGNED_INVERT)
                {
                    opcode = m ? "crn" : "add";
                    args = 0b1010;
                }
                else if (m)
                {
                    opcode = "mux";
                    args = 0b1111;
                }
                else if (b == NV_REG_ZERO && y == NV_MAPPING_UNSIGNED_INVERT && d == NV_REG_ZERO && w == NV_MAPPING_EXPAND_NORMAL)
                {
                    opcode = "sub";
                    args = 0b1010;
                }
                else if (d == NV_REG_ZERO && w == NV_MAPPING_UNSIGNED_INVERT)
                {
                    opcode = "mad";
                    args = 0b1110;
                }
                else if (a == c && x == NV_MAPPING_UNSIGNED_IDENTITY && y == NV_MAPPING_UNSIGNED_IDENTITY && z == NV_MAPPING_UNSIGNED_INVERT && w == NV_MAPPING_UNSIGNED_IDENTITY)
                {
                    opcode = "lrp";
                    args = 0b1101;
                }
                else
                {
                    opcode = "mma";
                    args = 0b1111;
                }
            };

            int args[3] = {};
            char const* opcode[3] = {};
            mma(opcode[1], args[1],
                colorOCW.sum.reg, colorOCW.sum.mux,
                colorICW.a.reg, colorICW.b.reg, colorICW.c.reg, colorICW.d.reg,
                colorICW.a.mapping, colorICW.b.mapping, colorICW.c.mapping, colorICW.d.mapping);
            mma(opcode[2], args[2],
                alphaOCW.sum.reg, alphaOCW.sum.mux,
                alphaICW.a.reg, alphaICW.b.reg, alphaICW.c.reg,
                alphaICW.d.reg, alphaICW.a.mapping, alphaICW.b.mapping, alphaICW.c.mapping, alphaICW.d.mapping);
            if (colorOCW.shift == alphaOCW.shift &&
                colorOCW.sum.reg == alphaOCW.sum.reg &&
                colorOCW.sum.mux == alphaOCW.sum.mux &&
                colorICW.a.reg == alphaICW.a.reg && colorICW.b.reg == alphaICW.b.reg &&
                colorICW.c.reg == alphaICW.c.reg && colorICW.d.reg == alphaICW.d.reg &&
                colorICW.a.mapping == alphaICW.a.mapping && colorICW.b.mapping == alphaICW.b.mapping &&
                colorICW.c.mapping == alphaICW.c.mapping && colorICW.d.mapping == alphaICW.d.mapping)
            {
                args[0] = args[1];
                opcode[0] = opcode[1];
                opcode[1] = opcode[2] = nullptr;
            }

            for (size_t i = 0; i < 3; ++i)
            {
                if (opcode[i] == nullptr)
                    continue;
                next();
                int o = (i != 2) ? colorOCW.sum.reg : alphaOCW.sum.reg;
                int s = (i != 2) ? colorOCW.shift : alphaOCW.shift;
                int a = (i != 2) ? colorICW.a.reg : alphaICW.a.reg;
                int b = (i != 2) ? colorICW.b.reg : alphaICW.b.reg;
                int c = (i != 2) ? colorICW.c.reg : alphaICW.c.reg;
                int d = (i != 2) ? colorICW.d.reg : alphaICW.d.reg;
                int x = (i != 2) ? colorICW.a.mapping : alphaICW.a.mapping;
                int y = (i != 2) ? colorICW.b.mapping : alphaICW.b.mapping;
                int z = (i != 2) ? colorICW.c.mapping : alphaICW.c.mapping;
                int w = (i != 2) ? colorICW.d.mapping : alphaICW.d.mapping;
                if (opcode[i])        { text += opcode[i]; text += shiftText[s]; text += ' '; text += reg(o, swizzleText[i], 0); }
                if (args[i] & 0b1000) { text += ',';                             text += ' '; text += reg(a, swizzleText[i], x); }
                if (args[i] & 0b0100) { text += ',';                             text += ' '; text += reg(b, swizzleText[i], y); }
                if (args[i] & 0b0010) { text += ',';                             text += ' '; text += reg(c, swizzleText[i], z); }
                if (args[i] & 0b0001) { text += ',';                             text += ' '; text += reg(d, swizzleText[i], w); }
            }
        }

        // DP3 / MUL
        if (true)
        {
            auto muldp3 = [&](int cd, int co, int cs, int ca, int cb, int cx, int cy,
                              int ad, int ao, int as, int aa, int ab, int ax, int ay)
            {
                int args[3] = {};
                char const* opcode[3] = {};
                char const* shift[3] = {};
                if (co != NV_REG_ZERO)
                {
                    args[1] = 0b11;
                    opcode[1] = cd ? "dp3" : "mul";
                    shift[1] = shiftText[cs];
                    if (cb == NV_REG_ZERO && cy == NV_MAPPING_UNSIGNED_INVERT)
                    {
                        args[1] = cd ? 0b11 : 0b10;
                        opcode[1] = cd ? "dp3" : "mov";
                    }
                }
                if (ao != NV_REG_ZERO)
                {
                    args[2] = 0b11;
                    opcode[2] = "mul";
                    shift[2] = shiftText[as];
                    if (ab == NV_REG_ZERO && ay == NV_MAPPING_UNSIGNED_INVERT)
                    {
                        args[2] = 0b10;
                        opcode[2] = "mov";
                    }
                }
                if (cd == ad && co == ao && cs == as)
                {
                    args[0] = args[1];
                    opcode[0] = opcode[1];
                    opcode[1] = opcode[2] = nullptr;
                    shift[0] = shift[1];
                }
                for (size_t i = 0; i < 3; ++i)
                {
                    if (opcode[i] == nullptr)
                        continue;
                    next();
                    int o = (i != 2) ? co : ao;
                    int a = (i != 2) ? ca : aa;
                    int b = (i != 2) ? cb : ab;
                    int x = (i != 2) ? cx : ax;
                    int y = (i != 2) ? cy : ay;
                    if (opcode[i])      { text += opcode[i]; text += shift[i]; text += ' '; text += reg(o, swizzleText[i], 0); }
                    if (args[i] & 0b10) { text += ',';                         text += ' '; text += reg(a, swizzleText[i], x); }
                    if (args[i] & 0b01) { text += ',';                         text += ' '; text += reg(b, swizzleText[i], y); }
                }
            };
            muldp3(colorOCW.ab.dp, colorOCW.ab.reg, colorOCW.shift, colorICW.a.reg, colorICW.b.reg, colorICW.a.mapping, colorICW.b.mapping,
                   alphaOCW.ab.dp, alphaOCW.ab.reg, alphaOCW.shift, alphaICW.a.reg, alphaICW.b.reg, alphaICW.a.mapping, alphaICW.b.mapping);
            muldp3(colorOCW.cd.dp, colorOCW.cd.reg, colorOCW.shift, colorICW.c.reg, colorICW.d.reg, colorICW.c.mapping, colorICW.d.mapping,
                   alphaOCW.cd.dp, alphaOCW.cd.reg, alphaOCW.shift, alphaICW.c.reg, alphaICW.d.reg, alphaICW.c.mapping, alphaICW.d.mapping);
        }

        text += '\n';
    }

    // Constant
    for (size_t i = 4 + dwStage * 4; i < code.size(); i += 4)
    {
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
