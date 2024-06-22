//==============================================================================
// Minamoto : ShaderAssemblerNV20 Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <assert.h>
#include <Tools/WindowsHeader.h>
#include "ShaderAssemblerNV20.h"

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
struct CPixelShaderPublic : public CPixelShader
{
    using CPixelShader::m_dwStage;
    using CPixelShader::m_cw;
    using CPixelShader::m_pixelShaderConsts;
    using CPixelShader::m_dwPixelShaderConstFlags;
    using CPixelShader::GetShaderProgram;
    using CPixelShader::PSProgramNames;
};
//------------------------------------------------------------------------------
struct RegisterCombinerInput
{
    struct Input
    {
        uint8_t reg:4;
        uint8_t alpha:1;
        uint8_t mapping:3;
    };
    Input d;
    Input c;
    Input b;
    Input a;
};
//------------------------------------------------------------------------------
union RegisterCombinerOutput
{
    struct
    {
        uint32_t reg:4;
        uint32_t __:8;
        uint32_t dp:1;
        uint32_t ___:5;
        uint32_t a:1;
        uint32_t ____:1;
        uint32_t only:1;
        uint32_t _____:3;
        uint32_t trunc:1;
    } cd;
    struct
    {
        uint32_t _:4;
        uint32_t reg:4;
        uint32_t __:5;
        uint32_t dp:1;
        uint32_t ___:5;
        uint32_t a:1;
        uint32_t ____:1;
        uint32_t only:1;
        uint32_t _____:3;
        uint32_t trunc:1;
    } ab;
    struct
    {
        uint32_t _:8;
        uint32_t reg:4;
        uint32_t __:2;
        uint32_t mux:1;
    } abcd;
    struct
    {
        uint32_t _:15;
        uint32_t shift:3;
    };
};
//------------------------------------------------------------------------------
static void dumpTextureShaders(uint32_t* cw, uint32_t stage)
{
    DPF_PLAIN("%12s ", "************");
    DPF_PLAIN("%s ", "Texture Shaders");
    DPF("%12s ", "************");

    DPF("%1s|%32s|%32s|%32s|", "", "2D", "3D", "CUBE");

    DPF_PLAIN("%1s|", "-");
    for (size_t i = 0; i < 3; ++i)
    {
        DPF_PLAIN("%32s|", "--------------------------------");
    }
    DPF("");

    for (size_t i = 0; i < stage; ++i)
    {
        DPF_PLAIN("%zd|", i);
        for (size_t j = 0; j < 3; ++j)
        {
            DPF_PLAIN("%32s|", CPixelShaderPublic::PSProgramNames[(cw[j] >> (i * 5)) & 0x1F]);
        }
        DPF("");
    }
}
//------------------------------------------------------------------------------
static void dumpRegisterCombinersCode(uint32_t* cw, uint32_t stage)
{
    DPF("%*s                         A C A C A C     A C   A C                                 A C", 4 + 9 + 9 + 9 + 9, "");
    DPF("%*sA     B     C     D      B D B D B D     B D   B D  A     B     C     D            B D", 4 + 9 + 9 + 9 + 9, "");
    DPF("%*sM A R M A R M A R M A R  T T O O A A S M D D R R R  M A R M A R M A R M A R  S M R R R", 4 + 9 + 9 + 9 + 9, "");

    for (size_t i = 0; i < stage; ++i)
    {
        RegisterCombinerInput colorICW = (RegisterCombinerInput&)cw[i * 4 + 0];
        RegisterCombinerOutput colorOCW = (RegisterCombinerOutput&)cw[i * 4 + 1];
        RegisterCombinerInput alphaICW = (RegisterCombinerInput&)cw[i * 4 + 2];
        RegisterCombinerOutput alphaOCW = (RegisterCombinerOutput&)cw[i * 4 + 3];

        DPF_PLAIN("%02x: ", i);
        DPF_PLAIN("%08x ", cw[i * 4 + 0]);
        DPF_PLAIN("%08x ", cw[i * 4 + 1]);
        DPF_PLAIN("%08x ", cw[i * 4 + 2]);
        DPF_PLAIN("%08x ", cw[i * 4 + 3]);

        DPF_PLAIN("%x %x %x %x %x %x %x %x %x %x %x %x ",
                  colorICW.a.mapping, colorICW.a.alpha, colorICW.a.reg,
                  colorICW.b.mapping, colorICW.b.alpha, colorICW.b.reg,
                  colorICW.c.mapping, colorICW.c.alpha, colorICW.c.reg,
                  colorICW.d.mapping, colorICW.d.alpha, colorICW.d.reg);

        DPF_PLAIN(" ");

        DPF_PLAIN("%x %x %x %x %x %x %x %x %x %x %x %x %x ",
                  colorOCW.ab.trunc, colorOCW.cd.trunc,
                  colorOCW.ab.only, colorOCW.cd.only,
                  colorOCW.ab.a, colorOCW.cd.a, colorOCW.shift,
                  colorOCW.abcd.mux, colorOCW.ab.dp, colorOCW.cd.dp,
                  colorOCW.abcd.reg, colorOCW.ab.reg, colorOCW.cd.reg);

        DPF_PLAIN(" ");

        DPF_PLAIN("%x %x %x %x %x %x %x %x %x %x %x %x ",
                  alphaICW.a.mapping, alphaICW.a.alpha, alphaICW.a.reg,
                  alphaICW.b.mapping, alphaICW.b.alpha, alphaICW.b.reg,
                  alphaICW.c.mapping, alphaICW.c.alpha, alphaICW.c.reg,
                  alphaICW.d.mapping, alphaICW.d.alpha, alphaICW.d.reg);

        DPF_PLAIN(" ");

        DPF("%x %x %x %x %x ",
            alphaOCW.shift,
            alphaOCW.abcd.mux,
            alphaOCW.abcd.reg, alphaOCW.ab.reg, alphaOCW.cd.reg);
    }
}
//------------------------------------------------------------------------------
static void dumpRegisterCombiners(uint32_t* cw, uint32_t stage)
{
    DPF_PLAIN("%12s ", "************");
    DPF_PLAIN("%s ", "Register Combiners");
    DPF("%12s ", "************");

    DPF_PLAIN("%18s|%18s|%18s|%18s|", "A", "B", "C", "D");
    DPF_PLAIN("%1s|", "");
    DPF("%18s|%18s|%18s|%18s|", "A", "B", "C", "D");

    for (size_t i = 0; i < stage; ++i)
    {
        RegisterCombinerInput colorICW = (RegisterCombinerInput&)cw[i * 4 + 0];
        RegisterCombinerOutput colorOCW = (RegisterCombinerOutput&)cw[i * 4 + 1];
        RegisterCombinerInput alphaICW = (RegisterCombinerInput&)cw[i * 4 + 2];
        RegisterCombinerOutput alphaOCW = (RegisterCombinerOutput&)cw[i * 4 + 3];

        DPF_PLAIN("%18s|", "------------------");
        DPF_PLAIN("%18s|", "------------------");
        DPF_PLAIN("%18s|", "------------------");
        DPF_PLAIN("%18s|", "------------------");
        DPF_PLAIN("%1s|", "-");
        DPF_PLAIN("%18s|", "------------------");
        DPF_PLAIN("%18s|", "------------------");
        DPF_PLAIN("%18s|", "------------------");
        DPF("%18s|", "------------------");

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
            "SPARE0_PLUS_CONSTANT1",
            "E_TIMES_F",
        };

        DPF_PLAIN("%18s|", colorICW.a.alpha ? "ALPHA" : "RGB");
        DPF_PLAIN("%18s|", colorICW.b.alpha ? "ALPHA" : "RGB");
        DPF_PLAIN("%18s|", colorICW.c.alpha ? "ALPHA" : "RGB");
        DPF_PLAIN("%18s|", colorICW.d.alpha ? "ALPHA" : "RGB");
        DPF_PLAIN("%1zd|", i);
        DPF_PLAIN("%18s|", alphaICW.a.alpha ? "ALPHA" : "BLUE");
        DPF_PLAIN("%18s|", alphaICW.b.alpha ? "ALPHA" : "BLUE");
        DPF_PLAIN("%18s|", alphaICW.c.alpha ? "ALPHA" : "BLUE");
        DPF("%18s|", alphaICW.d.alpha ? "ALPHA" : "BLUE");

        DPF_PLAIN("%18s|", reg[colorICW.a.reg]);
        DPF_PLAIN("%18s|", reg[colorICW.b.reg]);
        DPF_PLAIN("%18s|", reg[colorICW.c.reg]);
        DPF_PLAIN("%18s|", reg[colorICW.d.reg]);
        DPF_PLAIN("%1s|", "");
        DPF_PLAIN("%18s|", reg[alphaICW.a.reg]);
        DPF_PLAIN("%18s|", reg[alphaICW.b.reg]);
        DPF_PLAIN("%18s|", reg[alphaICW.c.reg]);
        DPF("%18s|", reg[alphaICW.d.reg]);

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

        DPF_PLAIN("%18s|", mapping[colorICW.a.mapping]);
        DPF_PLAIN("%18s|", mapping[colorICW.b.mapping]);
        DPF_PLAIN("%18s|", mapping[colorICW.c.mapping]);
        DPF_PLAIN("%18s|", mapping[colorICW.d.mapping]);
        DPF_PLAIN("%1s|", "");
        DPF_PLAIN("%18s|", mapping[alphaICW.a.mapping]);
        DPF_PLAIN("%18s|", mapping[alphaICW.b.mapping]);
        DPF_PLAIN("%18s|", mapping[alphaICW.c.mapping]);
        DPF("%18s|", mapping[alphaICW.d.mapping]);

        char temp[256];
        auto op = [&temp](char const* x, char const* op, char const* y, int s, int o, int a) -> char const*
        {
            char const* format = "";
            switch (s)
            {
            case 0: format = "(%s %s %s)";                  break;
            case 1: format = "((%s %s %s - 0.5)";           break;
            case 2: format = "((%s %s %s) * 2.0)";          break;
            case 3: format = "((%s %s %s - 0.5) * 2.0)";    break;
            case 4: format = "((%s %s %s) * 4.0)";          break;
            case 5: format = "((%s %s %s - 0.5) * 4.0)";    break;
            case 6: format = "((%s %s %s) / 2.0)";          break;
            case 7: format = "((%s %s %s - 0.5) / 2.0)";    break;
            }
            snprintf(temp, 256, format, x, op, y);
            if (o != NV_REG_ZERO)
            {
                strcat(temp, " = ");
                strcat(temp, reg[o]);
                if (a)
                {
                    strcat(temp, " ");
                    strcat(temp, "AND ALPHA");
                }
            }
            return temp;
        };

        std::string cab = op("A", colorOCW.ab.dp ? "DOT" : "MUL", "B", colorOCW.shift, colorOCW.ab.reg, colorOCW.ab.a);
        std::string ccd = op("C", colorOCW.cd.dp ? "DOT" : "MUL", "D", colorOCW.shift, colorOCW.cd.reg, colorOCW.cd.a);
        std::string aab = op("A", alphaOCW.ab.dp ? "DOT" : "MUL", "B", alphaOCW.shift, alphaOCW.ab.reg, 0);
        std::string acd = op("C", alphaOCW.cd.dp ? "DOT" : "MUL", "D", alphaOCW.shift, alphaOCW.cd.reg, 0);

        DPF_PLAIN("%37s|", colorOCW.ab.reg ? cab.c_str() : "NOP");
        DPF_PLAIN("%37s|", colorOCW.cd.reg ? ccd.c_str() : "NOP");
        DPF_PLAIN("%1s|", "");
        DPF_PLAIN("%37s|", alphaOCW.ab.reg ? aab.c_str() : "NOP");
        DPF("%37s|", alphaOCW.cd.reg ? acd.c_str() : "NOP");

        cab = cab.substr(0, cab.find(" = "));
        ccd = ccd.substr(0, ccd.find(" = "));
        aab = aab.substr(0, aab.find(" = "));
        acd = acd.substr(0, acd.find(" = "));

        std::string cabcd = op(cab.c_str(), colorOCW.abcd.mux ? "MUX" : "ADD", ccd.c_str(), colorOCW.shift, colorOCW.abcd.reg, 0);
        std::string aabcd = op(aab.c_str(), alphaOCW.abcd.mux ? "MUX" : "ADD", acd.c_str(), alphaOCW.shift, alphaOCW.abcd.reg, 0);

        DPF_PLAIN("%75s|", colorOCW.abcd.reg ? cabcd.c_str() : "NOP");
        DPF_PLAIN("%1s|", "");
        DPF("%75s|", alphaOCW.abcd.reg ? aabcd.c_str() : "NOP");
    }
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

            CPixelShaderPublic pixelShader;
            pixelShader.create(nullptr, 0, (DWORD)shader.size(), (DWORD*)shader.data());

            uint32_t combinerControl = 0;
            combinerControl |= DRF_NUM(097, _SET_COMBINER_CONTROL, _ITERATION_COUNT, pixelShader.m_dwStage);
            combinerControl |= DRF_DEF(097, _SET_COMBINER_CONTROL, _FACTOR0, _EACH_STAGE);
            combinerControl |= DRF_DEF(097, _SET_COMBINER_CONTROL, _FACTOR1, _EACH_STAGE);
            combinerControl |= DRF_DEF(097, _SET_COMBINER_CONTROL, _MUX_SELECT, _MSB);

            uint32_t shaderPrograms[4] = {};
            for (DWORD i = 1; i < 4; ++i)
            {
                shaderPrograms[i] |= DRF_NUM(097, _SET_SHADER_STAGE_PROGRAM, _STAGE0, pixelShader.GetShaderProgram(nullptr, 0, i));
                shaderPrograms[i] |= DRF_NUM(097, _SET_SHADER_STAGE_PROGRAM, _STAGE1, pixelShader.GetShaderProgram(nullptr, 1, i));
                shaderPrograms[i] |= DRF_NUM(097, _SET_SHADER_STAGE_PROGRAM, _STAGE2, pixelShader.GetShaderProgram(nullptr, 2, i));
                shaderPrograms[i] |= DRF_NUM(097, _SET_SHADER_STAGE_PROGRAM, _STAGE3, pixelShader.GetShaderProgram(nullptr, 3, i));
            }

            code.push_back(combinerControl);
            code.push_back(shaderPrograms[1]);
            code.push_back(shaderPrograms[2]);
            code.push_back(shaderPrograms[3]);

            for (size_t i = 0; i < pixelShader.m_dwStage; ++i)
            {
                code.push_back(pixelShader.m_cw[i][PSHAD_COLOR][PSHAD_ICW]);
                code.push_back(pixelShader.m_cw[i][PSHAD_COLOR][PSHAD_OCW]);
                code.push_back(pixelShader.m_cw[i][PSHAD_ALPHA][PSHAD_ICW]);
                code.push_back(pixelShader.m_cw[i][PSHAD_ALPHA][PSHAD_OCW]);
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

            dumpRegisterCombinersCode(code.data() + 4, pixelShader.m_dwStage);

            dumpTextureShaders(shaderPrograms + 1, 4);
            dumpRegisterCombiners(code.data() + 4, pixelShader.m_dwStage);
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
std::string ShaderAssemblerNV20::DisassembleKelvin(std::vector<uint32_t> const& code) __attribute__((optnone))
{
    char temp[256];
    std::string text;

    // ???
    if (code.size() < 4)
        return text;

    // Stage
    size_t stage = uint8_t(code.front());
    if (code.size() < (4 + stage * 4))
        return text;

    // Dump
    int line = 0;
    auto next = [&]()
    {
        if (line)
        {
            text += '\n';
            text.append(5 + 9 * 4, ' ');
            text += '+';
        }
        line++;
    };

    // Stage
    for (size_t i = 0; i < 4; ++i)
    {
        snprintf(temp, 256, "%3zd: ", i);
        text += temp;
        snprintf(temp, 256, "%08X ", code[i]);
        text += temp;

        for (size_t j = 0; j < 3; ++j)
        {
            snprintf(temp, 256, "%9s", "");
            text += temp;
        }

        switch (i)
        {
        case 0: 
            snprintf(temp, 256, "%-9s ", "control");
            break;
        case 1:
            snprintf(temp, 256, "%-9s ", "2d");
            break;
        case 2:
            snprintf(temp, 256, "%-9s ", "3d");
            break;
        case 3:
            snprintf(temp, 256, "%-9s ", "cube");
            break;
        }
        text += temp;

        uint32_t program = code[i];
        for (size_t j = 0; j < 8; ++j)
        {
            if (program == 0)
                break;
            if (j) text += ", ";
            if (i == 0)
            {
                if (j == 1)
                    snprintf(temp, 256, "%s", program & 0x1 ? "msb" : "lsb");
                else
                    snprintf(temp, 256, "%d", program & 0xF);
                program >>= (j == 0) ? 8 : 4;
            }
            else
            {
                snprintf(temp, 256, "%d", program & 0x1F);
                program >>= 5;
            }
            text += temp;
        }

        text += '\n';
    }

    // Register Combiner
    for (size_t i = 4; i < (4 + stage * 4); i += 4)
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

        static char const* const colorSwizzleText[4] =
        {
            nullptr,
            "rgb",
            "a",
        };
        static char const* const alphaSwizzleText[4] =
        {
            nullptr,
            "b",
            "a",
        };
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

        // Reset
        line = 0;

        // MMA
        if (true)
        {
            auto mma = [](RegisterCombinerOutput output, RegisterCombinerInput input, int& args) -> char const*
            {
                if (output.abcd.reg == NV_REG_ZERO)
                    return nullptr;

                // Signed type
                if (input.a.mapping == NV_MAPPING_UNSIGNED_IDENTITY)
                    input.a.mapping = NV_MAPPING_SIGNED_IDENTITY;
                if (input.b.mapping == NV_MAPPING_UNSIGNED_IDENTITY)
                    input.b.mapping = NV_MAPPING_SIGNED_IDENTITY;
                if (input.c.mapping == NV_MAPPING_UNSIGNED_IDENTITY)
                    input.c.mapping = NV_MAPPING_SIGNED_IDENTITY;
                if (input.d.mapping == NV_MAPPING_UNSIGNED_IDENTITY)
                    input.d.mapping = NV_MAPPING_SIGNED_IDENTITY;

                // Instruction
                if (input.b.reg == NV_REG_ZERO && input.b.mapping == NV_MAPPING_UNSIGNED_INVERT &&
                    input.d.reg == NV_REG_ZERO && input.d.mapping == NV_MAPPING_UNSIGNED_INVERT)
                {
                    args = 0b1010;
                    return output.abcd.mux ? "crn" : "add";
                }
                if (output.abcd.mux)
                {
                    args = 0b1111;
                    return "mux";
                }
                if (input.b.reg == NV_REG_ZERO && input.b.mapping == NV_MAPPING_UNSIGNED_INVERT &&
                    input.d.reg == NV_REG_ZERO && input.d.mapping == NV_MAPPING_EXPAND_NORMAL)
                {
                    args = 0b1010;
                    return "sub";
                }
                if (input.d.reg == NV_REG_ZERO && input.d.mapping == NV_MAPPING_UNSIGNED_INVERT)
                {
                    args = 0b1110;
                    return "mad";
                }
                if (input.a.reg == input.c.reg &&
                    input.a.mapping == NV_MAPPING_SIGNED_IDENTITY && input.b.mapping == NV_MAPPING_SIGNED_IDENTITY &&
                    input.c.mapping == NV_MAPPING_UNSIGNED_INVERT && input.d.mapping == NV_MAPPING_SIGNED_IDENTITY)
                {
                    args = 0b1101;
                    return "lrp";
                }
                args = 0b1111;
                return "mma";
            };

            int args[3] = {};
            char const* opcode[3] = {};
            opcode[1] = mma(colorOCW, colorICW, args[1]);
            opcode[2] = mma(alphaOCW, alphaICW, args[2]);
            if (colorOCW.shift == alphaOCW.shift &&
                colorOCW.abcd.reg == alphaOCW.abcd.reg &&
                colorOCW.abcd.mux == alphaOCW.abcd.mux &&
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
                int o = (i != 2) ? colorOCW.abcd.reg : alphaOCW.abcd.reg;
                int s = (i != 2) ? colorOCW.shift : alphaOCW.shift;
                int a = (i != 2) ? colorICW.a.reg : alphaICW.a.reg;
                int b = (i != 2) ? colorICW.b.reg : alphaICW.b.reg;
                int c = (i != 2) ? colorICW.c.reg : alphaICW.c.reg;
                int d = (i != 2) ? colorICW.d.reg : alphaICW.d.reg;
                int x = (i != 2) ? colorICW.a.mapping : alphaICW.a.mapping;
                int y = (i != 2) ? colorICW.b.mapping : alphaICW.b.mapping;
                int z = (i != 2) ? colorICW.c.mapping : alphaICW.c.mapping;
                int w = (i != 2) ? colorICW.d.mapping : alphaICW.d.mapping;
                char const* swizzle = (i != 2) ? colorSwizzleText[i] : alphaSwizzleText[i];
                char const* swizzleX = (i != 2) ? colorSwizzleText[colorICW.a.alpha ? 2 : i] : alphaSwizzleText[alphaICW.a.alpha ? 2 : i];
                char const* swizzleY = (i != 2) ? colorSwizzleText[colorICW.b.alpha ? 2 : i] : alphaSwizzleText[alphaICW.b.alpha ? 2 : i];
                char const* swizzleZ = (i != 2) ? colorSwizzleText[colorICW.c.alpha ? 2 : i] : alphaSwizzleText[alphaICW.c.alpha ? 2 : i];
                char const* swizzleW = (i != 2) ? colorSwizzleText[colorICW.d.alpha ? 2 : i] : alphaSwizzleText[alphaICW.d.alpha ? 2 : i];
                snprintf(temp, 256, "%-9s ", (std::string(opcode[i]) + shiftText[s]).c_str());
                if (temp[0])          { text += temp;              text += reg(o, swizzle, 0); }
                if (args[i] & 0b1000) { text += ',';  text += ' '; text += reg(a, swizzleX, x); }
                if (args[i] & 0b0100) { text += ',';  text += ' '; text += reg(b, swizzleY, y); }
                if (args[i] & 0b0010) { text += ',';  text += ' '; text += reg(c, swizzleZ, z); }
                if (args[i] & 0b0001) { text += ',';  text += ' '; text += reg(d, swizzleW, w); }
            }
        }

        // DP3 / MUL
        if (true)
        {
            auto muldp3 = [&](int cdot, int cdst, int cshift, int ctoa, int caonly, RegisterCombinerInput::Input ca, RegisterCombinerInput::Input cb,
                              int adot, int adst, int ashift, int,      int,        RegisterCombinerInput::Input aa, RegisterCombinerInput::Input ab)
            {
                int args[3] = {};
                char const* opcode[3] = {};
                char const* shift[3] = {};
                char const* swizzle[3] = {};
                char const* swizzleX[3] = {};
                char const* swizzleY[3] = {};
                if (cdst != NV_REG_ZERO)
                {
                    args[1] = 0b11;
                    opcode[1] = cdot ? "dp3" : "mul";
                    shift[1] = shiftText[cshift];
                    swizzle[1] = colorSwizzleText[1];
                    swizzleX[1] = colorSwizzleText[ca.alpha ? 2 : 1];
                    swizzleY[1] = colorSwizzleText[cb.alpha ? 2 : 1];
                    if (cb.reg == NV_REG_ZERO && cb.mapping == NV_MAPPING_UNSIGNED_INVERT)
                    {
                        args[1] = cdot ? 0b11 : 0b10;
                        opcode[1] = cdot ? "dp3" : "mov";
                    }
                    if (cdot && ctoa)
                    {
                        swizzle[1] = caonly ? "a" : "rgba";
                    }
                }
                if (adst != NV_REG_ZERO)
                {
                    args[2] = 0b11;
                    opcode[2] = "mul";
                    shift[2] = shiftText[ashift];
                    swizzle[2] = alphaSwizzleText[2];
                    swizzleX[2] = alphaSwizzleText[aa.alpha ? 2 : 1];
                    swizzleY[2] = alphaSwizzleText[ab.alpha ? 2 : 1];
                    if (ab.reg == NV_REG_ZERO && ab.mapping == NV_MAPPING_UNSIGNED_INVERT)
                    {
                        args[2] = 0b10;
                        opcode[2] = "mov";
                    }
                }
                if (cdot == adot && cdst == adst && cshift == ashift &&
                    ca.reg == aa.reg && cb.reg == ab.reg &&
                    ca.mapping == aa.mapping && cb.mapping == ab.mapping)
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
                    int o = (i != 2) ? cdst : adst;
                    int a = (i != 2) ? ca.reg : aa.reg;
                    int b = (i != 2) ? cb.reg : ab.reg;
                    int x = (i != 2) ? ca.mapping : aa.mapping;
                    int y = (i != 2) ? cb.mapping : ab.mapping;
                    snprintf(temp, 256, "%-9s ", (std::string(opcode[i]) + shift[i]).c_str());
                    if (temp[0])        { text += temp;              text += reg(o, swizzle[i], 0); }
                    if (args[i] & 0b10) { text += ',';  text += ' '; text += reg(a, swizzleX[i], x); }
                    if (args[i] & 0b01) { text += ',';  text += ' '; text += reg(b, swizzleY[i], y); }
                }
            };
            muldp3(colorOCW.ab.dp, colorOCW.ab.reg, colorOCW.shift, colorOCW.ab.a, colorOCW.ab.only, colorICW.a, colorICW.b,
                   alphaOCW.ab.dp, alphaOCW.ab.reg, alphaOCW.shift, 0,             0,                alphaICW.a, alphaICW.b);
            muldp3(colorOCW.cd.dp, colorOCW.cd.reg, colorOCW.shift, colorOCW.cd.a, colorOCW.cd.only, colorICW.c, colorICW.d,
                   alphaOCW.cd.dp, alphaOCW.cd.reg, alphaOCW.shift, 0,             0,                alphaICW.c, alphaICW.d);
        }

        text += '\n';
    }

    // Constant
    for (size_t i = 4 + stage * 4; i < code.size(); i += 4)
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
