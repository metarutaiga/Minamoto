//==============================================================================
// Minamoto : ShaderAssemblerD3D8 Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <Tools/WindowsHeader.h>
#include "ShaderAssemblerD3D8.h"

#include <dxsdk/d3d8types.h>
#include <dxsdk/d3d8caps.h>
#if (DIRECT3D_VERSION >= 0x0900)
#define D3DSIO_TEXM3x3DIFF  (D3DSIO_TEXM3x3TEX + 1)
typedef int D3DSHADER_PARAM_DSTMOD_TYPE;
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wextra-tokens"
#pragma clang diagnostic ignored "-Wmultichar"
#endif

#include <shval/errlog.hpp>
#include <shval/valbase.hpp>
#include <shval/pshdrval.hpp>
#include <shval/vshdrval.hpp>

//==============================================================================
std::vector<uint32_t> ShaderAssemblerD3D8::Assemble(std::string const& shader, std::string& message)
{
    std::string_view line;
    auto token = [&line]()
    {
        if (line.empty())
            return std::string_view();
        size_t prefix = line.find_first_not_of(", \t");
        if (prefix != std::string_view::npos)
            line.remove_prefix(prefix);
        size_t suffix = line.find_first_of(", \t");
        if (suffix == std::string_view::npos)
            suffix = line.size();
        std::string_view output = line.substr(0, suffix);
        line.remove_prefix(suffix);
        return output;
    };

    std::vector<uint32_t> code;
    auto reg = [&code](std::string_view view, bool source, int shift, bool saturate)
    {
        if (view.empty() || view[0] == ';' || view[0] == '/')
            return false;
        bool dot = false;
        bool underline = false;
        uint64_t typeShift = 0;
        uint64_t maskShift = 0;
        uint64_t modShift = 0;
        uint64_t type = 0;
        uint64_t index = 0;
        uint64_t mask = 0;
        uint64_t mod = 0;
        for (uint64_t c : view)
        {
            if (c == '.')
            {
                dot = true;
                underline = false;
            }
            else if (c == '_')
            {
                underline = true;
            }
            else if (c == '-')
            {
                mod = mod | (c << modShift);
                modShift += 8;
            }
            else if (c >= '0' && c <= '9')
            {
                if (underline || type == 0)
                {
                    mod = mod | (c << modShift);
                    modShift += 8;
                    continue;
                }
                index = (index * 10) + (c - '0');
            }
            else if (c >= 'a' && c <= 'z')
            {
                if (dot)
                {
                    switch (c)
                    {
                    case 'r':
                    case 'g':
                    case 'b':
                    case 'a':
                    case 'x':
                    case 'y':
                    case 'z':
                    case 'w':
                        mask = (mask & ~(0xFFFFFFFF << maskShift)) | (c * (0x01010101 << maskShift));
                        maskShift += 8;
                        break;
                    }
                    continue;
                }
                if (underline)
                {
                    mod = mod | (c << modShift);
                    modShift += 8;
                    continue;
                }
                type = type | (c << typeShift);
                typeShift += 8;
            }
        }
        uint32_t opcode = 0x80000000;
        switch (type)
        {
        case "r"_cc:    opcode |= D3DSPR_TEMP;                  break;
        case "v"_cc:    opcode |= D3DSPR_INPUT;                 break;
        case "c"_cc:    opcode |= D3DSPR_CONST;                 break;
        case "a"_cc:    opcode |= D3DSPR_ADDR;                  break;
        case "t"_cc:    opcode |= D3DSPR_TEXTURE;               break;
        case "opos"_cc: opcode |= D3DSPR_RASTOUT;   index = 0;  break;
        case "ofog"_cc: opcode |= D3DSPR_RASTOUT;   index = 1;  break;
        case "opts"_cc: opcode |= D3DSPR_RASTOUT;   index = 2;  break;
        case "od"_cc:   opcode |= D3DSPR_ATTROUT;               break;
        case "ot"_cc:   opcode |= D3DSPR_TEXCRDOUT;             break;
        }
        opcode |= index;
        if (source)
        {
            switch (mod)
            {
            case "-"_CC:    opcode |= D3DSPSM_NEG;      break;
            case "bias"_CC: opcode |= D3DSPSM_BIAS;     break;
            case "-bias"_CC:opcode |= D3DSPSM_BIASNEG;  break;
            case "bx2"_CC:  opcode |= D3DSPSM_SIGN;     break;
            case "-bx2"_CC: opcode |= D3DSPSM_SIGNNEG;  break;
            case "1-"_CC:   opcode |= D3DSPSM_COMP;     break;
            case "x2"_CC:   opcode |= D3DSPSM_X2;       break;
            case "-x2"_CC:  opcode |= D3DSPSM_X2NEG;    break;
            case "dz"_CC:
            case "db"_CC:   opcode |= D3DSPSM_DZ;       break;
            case "dw"_CC:
            case "da"_CC:   opcode |= D3DSPSM_DW;       break;
            }
        }
        else
        {
            opcode |= (shift << D3DSP_DSTSHIFT_SHIFT);
            if (saturate)
            {
                opcode |= D3DSPDM_SATURATE;
            }
        }
        if (mask)
        {
            for (size_t i = 0; i < 4; ++i)
            {
                switch (uint8_t(mask))
                {
                case 'r': case 'x': opcode |= source ? (0 << (D3DSP_SWIZZLE_SHIFT + i * 2)) : D3DSP_WRITEMASK_0; break;
                case 'g': case 'y': opcode |= source ? (1 << (D3DSP_SWIZZLE_SHIFT + i * 2)) : D3DSP_WRITEMASK_1; break;
                case 'b': case 'z': opcode |= source ? (2 << (D3DSP_SWIZZLE_SHIFT + i * 2)) : D3DSP_WRITEMASK_2; break;
                case 'a': case 'w': opcode |= source ? (3 << (D3DSP_SWIZZLE_SHIFT + i * 2)) : D3DSP_WRITEMASK_3; break;
                }
                mask >>= 8;
            }
        }
        else
        {
            opcode |= source ? D3DSP_NOSWIZZLE : D3DSP_WRITEMASK_ALL;
        }
        code.push_back(opcode);
        return true;
    };

    // LowerCase
    std::string shaderLowerCase;
    shaderLowerCase.reserve(shader.size());
    for (char c : shader)
    {
        if (c >= 'A' && c <= 'Z')
            c += 0x20;
        shaderLowerCase.push_back(c);
    }

    // Assembly
    size_t begin = 0;
    while (begin < shaderLowerCase.size())
    {
        size_t end = shaderLowerCase.find("\n", begin);
        if (end == std::string::npos)
            end = shaderLowerCase.size();
        line = std::string_view(shaderLowerCase.data() + begin, end - begin);
        begin = end + 1;

        // Skip
        std::string_view first = token();
        if (first.empty() || first[0] == ';' || first[0] == '/')
            continue;

        // Co-issue
        bool coissue = false;
        if (first[0] == '+')
        {
            first.remove_prefix(1);
            coissue = true;
        }

        // Saturate
        bool saturate = false;
        if (first.find("_sat") != std::string_view::npos) { first.remove_suffix(4); saturate = true; }

        // Shift
        int shift = 0;
        if (first.find("_x2") != std::string_view::npos) { first.remove_suffix(3); shift = 1; }
        if (first.find("_x4") != std::string_view::npos) { first.remove_suffix(3); shift = 2; }
        if (first.find("_x8") != std::string_view::npos) { first.remove_suffix(3); shift = 3; }
        if (first.find("_d2") != std::string_view::npos) { first.remove_suffix(3); shift = 15; }
        if (first.find("_d4") != std::string_view::npos) { first.remove_suffix(3); shift = 14; }
        if (first.find("_d8") != std::string_view::npos) { first.remove_suffix(3); shift = 13; }

        // Opcode
        uint32_t opcode = D3DSIO_END;
        uint32_t dcl = UINT32_MAX;
        switch (xxHash(first.data(), first.size()))
        {
        case xxHash("vs_1_0"):
        case xxHash("vs.1.0"):          opcode = D3DVS_VERSION(1, 0);   break;
        case xxHash("vs_1_1"):
        case xxHash("vs.1.1"):          opcode = D3DVS_VERSION(1, 1);   break;
        case xxHash("ps_1_0"):
        case xxHash("ps.1.0"):          opcode = D3DPS_VERSION(1, 0);   break;
        case xxHash("ps_1_1"):
        case xxHash("ps.1.1"):          opcode = D3DPS_VERSION(1, 1);   break;
        case xxHash("ps_1_2"):
        case xxHash("ps.1.2"):          opcode = D3DPS_VERSION(1, 2);   break;
        case xxHash("ps_1_3"):
        case xxHash("ps.1.3"):          opcode = D3DPS_VERSION(1, 3);   break;
        case xxHash("ps_1_4"):
        case xxHash("ps.1.4"):          opcode = D3DPS_VERSION(1, 4);   break;
        case xxHash("nop"):             opcode = D3DSIO_NOP;            break;
        case xxHash("mov"):             opcode = D3DSIO_MOV;            break;
        case xxHash("add"):             opcode = D3DSIO_ADD;            break;
        case xxHash("sub"):             opcode = D3DSIO_SUB;            break;
        case xxHash("mad"):             opcode = D3DSIO_MAD;            break;
        case xxHash("mul"):             opcode = D3DSIO_MUL;            break;
        case xxHash("rcp"):             opcode = D3DSIO_RCP;            break;
        case xxHash("rsq"):             opcode = D3DSIO_RSQ;            break;
        case xxHash("dp3"):             opcode = D3DSIO_DP3;            break;
        case xxHash("dp4"):             opcode = D3DSIO_DP4;            break;
        case xxHash("min"):             opcode = D3DSIO_MIN;            break;
        case xxHash("max"):             opcode = D3DSIO_MAX;            break;
        case xxHash("slt"):             opcode = D3DSIO_SLT;            break;
        case xxHash("sge"):             opcode = D3DSIO_SGE;            break;
        case xxHash("exp"):             opcode = D3DSIO_EXP;            break;
        case xxHash("log"):             opcode = D3DSIO_LOG;            break;
        case xxHash("lit"):             opcode = D3DSIO_LIT;            break;
        case xxHash("dst"):             opcode = D3DSIO_DST;            break;
        case xxHash("lrp"):             opcode = D3DSIO_LRP;            break;
        case xxHash("frc"):             opcode = D3DSIO_FRC;            break;
        case xxHash("m4x4"):            opcode = D3DSIO_M4x4;           break;
        case xxHash("m4x3"):            opcode = D3DSIO_M4x3;           break;
        case xxHash("m3x4"):            opcode = D3DSIO_M3x4;           break;
        case xxHash("m3x3"):            opcode = D3DSIO_M3x3;           break;
        case xxHash("m3x2"):            opcode = D3DSIO_M3x2;           break;
#if (DIRECT3D_VERSION >= 0x0900)
        case xxHash("call"):            opcode = D3DSIO_CALL;           break;
        case xxHash("callnz"):          opcode = D3DSIO_CALLNZ;         break;
        case xxHash("loop"):            opcode = D3DSIO_LOOP;           break;
        case xxHash("ret"):             opcode = D3DSIO_RET;            break;
        case xxHash("endloop"):         opcode = D3DSIO_ENDLOOP;        break;
        case xxHash("label"):           opcode = D3DSIO_LABEL;          break;
        case xxHash("dcl"):             opcode = D3DSIO_DCL;            break;
        case xxHash("dcl_position"):    opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_POSITION;       break;
        case xxHash("dcl_blendweight"): opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_BLENDWEIGHT;    break;
        case xxHash("dcl_blendindices"):opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_BLENDINDICES;   break;
        case xxHash("dcl_normal"):      opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_NORMAL;         break;
        case xxHash("dcl_psize"):       opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_PSIZE;          break;
        case xxHash("dcl_texcoord"):    opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_TEXCOORD;       break;
        case xxHash("dcl_tangent"):     opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_TANGENT;        break;
        case xxHash("dcl_binormal"):    opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_BINORMAL;       break;
        case xxHash("dcl_tessfactor"):  opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_TESSFACTOR;     break;
        case xxHash("dcl_positiont"):   opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_POSITIONT;      break;
        case xxHash("dcl_color"):       opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_COLOR;          break;
        case xxHash("dcl_fog"):         opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_FOG;            break;
        case xxHash("dcl_depth"):       opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_DEPTH;          break;
        case xxHash("dcl_sample"):      opcode = D3DSIO_DCL; dcl = D3DDECLUSAGE_SAMPLE;         break;
        case xxHash("pow"):             opcode = D3DSIO_POW;            break;
        case xxHash("crs"):             opcode = D3DSIO_CRS;            break;
        case xxHash("sgn"):             opcode = D3DSIO_SGN;            break;
        case xxHash("abs"):             opcode = D3DSIO_ABS;            break;
        case xxHash("nrm"):             opcode = D3DSIO_NRM;            break;
        case xxHash("sincos"):          opcode = D3DSIO_SINCOS;         break;
        case xxHash("rep"):             opcode = D3DSIO_REP;            break;
        case xxHash("endrep"):          opcode = D3DSIO_ENDREP;         break;
        case xxHash("if"):              opcode = D3DSIO_IF;             break;
        case xxHash("ifc"):             opcode = D3DSIO_IFC;            break;
        case xxHash("else"):            opcode = D3DSIO_ELSE;           break;
        case xxHash("endif"):           opcode = D3DSIO_ENDIF;          break;
        case xxHash("break"):           opcode = D3DSIO_BREAK;          break;
        case xxHash("breakc"):          opcode = D3DSIO_BREAKC;         break;
        case xxHash("mova"):            opcode = D3DSIO_MOVA;           break;
        case xxHash("defb"):            opcode = D3DSIO_DEFB;           break;
        case xxHash("defi"):            opcode = D3DSIO_DEFI;           break;
#endif
        case xxHash("texcoord"):
        case xxHash("texcrd"):          opcode = D3DSIO_TEXCOORD;       break;
        case xxHash("texkill"):         opcode = D3DSIO_TEXKILL;        break;
        case xxHash("tex"):
        case xxHash("texld"):           opcode = D3DSIO_TEX;            break;
        case xxHash("texbem"):          opcode = D3DSIO_TEXBEM;         break;
        case xxHash("texbeml"):         opcode = D3DSIO_TEXBEML;        break;
        case xxHash("texreg2ar"):       opcode = D3DSIO_TEXREG2AR;      break;
        case xxHash("texreg2gb"):       opcode = D3DSIO_TEXREG2GB;      break;
        case xxHash("texm3x2pad"):      opcode = D3DSIO_TEXM3x2PAD;     break;
        case xxHash("texm3x2tex"):      opcode = D3DSIO_TEXM3x2TEX;     break;
        case xxHash("texm3x3pad"):      opcode = D3DSIO_TEXM3x3PAD;     break;
        case xxHash("texm3x3tex"):      opcode = D3DSIO_TEXM3x3TEX;     break;
        case xxHash("texm3x3diff"):     opcode = D3DSIO_TEXM3x3DIFF;    break;
        case xxHash("texm3x3spec"):     opcode = D3DSIO_TEXM3x3SPEC;    break;
        case xxHash("texm3x3vspec"):    opcode = D3DSIO_TEXM3x3VSPEC;   break;
        case xxHash("expp"):            opcode = D3DSIO_EXPP;           break;
        case xxHash("logp"):            opcode = D3DSIO_LOGP;           break;
        case xxHash("cnd"):             opcode = D3DSIO_CND;            break;
        case xxHash("def"):             opcode = D3DSIO_DEF;            break;
        case xxHash("texreg2rgb"):      opcode = D3DSIO_TEXREG2RGB;     break;
        case xxHash("texdp3tex"):       opcode = D3DSIO_TEXDP3TEX;      break;
        case xxHash("texm3x2depth"):    opcode = D3DSIO_TEXM3x2DEPTH;   break;
        case xxHash("texdp3"):          opcode = D3DSIO_TEXDP3;         break;
        case xxHash("texm3x3"):         opcode = D3DSIO_TEXM3x3;        break;
        case xxHash("texdepth"):        opcode = D3DSIO_TEXDEPTH;       break;
        case xxHash("cmp"):             opcode = D3DSIO_CMP;            break;
        case xxHash("bem"):             opcode = D3DSIO_BEM;            break;
#if (DIRECT3D_VERSION >= 0x0900)
        case xxHash("dp2add"):          opcode = D3DSIO_DP2ADD;         break;
        case xxHash("dsx"):             opcode = D3DSIO_DSX;            break;
        case xxHash("dsy"):             opcode = D3DSIO_DSY;            break;
        case xxHash("texldd"):          opcode = D3DSIO_TEXLDD;         break;
        case xxHash("setp"):            opcode = D3DSIO_SETP;           break;
        case xxHash("texldl"):          opcode = D3DSIO_TEXLDL;         break;
        case xxHash("breakp"):          opcode = D3DSIO_BREAKP;         break;
#endif
        case xxHash("phase"):           opcode = D3DSIO_PHASE;          break;
        case xxHash("end"):             opcode = D3DSIO_END;            break;
        default:
            message += first;
            message += '\n';
            break;
        }
        if (coissue)
        {
            opcode |= D3DSI_COISSUE;
        }
        code.push_back(opcode);
        if (dcl != UINT32_MAX)
        {
            code.push_back(dcl | 0x80000000);
        }

        // Constant
        if (opcode == D3DSIO_DEF)
        {
            reg(token(), false, 0, false);
            for (size_t i = 0; i < 4; ++i)
            {
                std::string_view view = token();
                float value = 0.0f;
                if (view.empty() == false)
                {
                    char* end = (char*)view.data() + view.size();
                    value = strtof(view.data(), &end);
                }
                code.push_back((uint32_t&)value);
            }
            continue;
        }

        reg(token(), false, shift, saturate);
        if (reg(token(), true, 0, false) == false)
            continue;
        if (reg(token(), true, 0, false) == false)
            continue;
        if (reg(token(), true, 0, false) == false)
            continue;
    }
    code.push_back(D3DSIO_END);

    // Validator
    if (code.empty() == false)
    {
        D3DCAPS8 caps = {};
        caps.VertexShaderVersion = D3DVS_VERSION(1, 1);
        caps.MaxVertexShaderConst = 96;
        caps.PixelShaderVersion = D3DPS_VERSION(1, 4);
        caps.MaxPixelShaderValue = 8.0f;

        CBaseShaderValidator* validator = nullptr;
        switch (code[0])
        {
        case D3DVS_VERSION(1, 0):
        case D3DVS_VERSION(1, 1):
            validator = new CVShaderValidator((DWORD*)code.data(), nullptr, &caps, SHADER_VALIDATOR_LOG_ERRORS);
            break;
        case D3DPS_VERSION(1, 0):
        case D3DPS_VERSION(1, 1):
        case D3DPS_VERSION(1, 2):
        case D3DPS_VERSION(1, 3):
            validator = new CPShaderValidator10((DWORD*)code.data(), &caps, SHADER_VALIDATOR_LOG_ERRORS);
            break;
        case D3DPS_VERSION(1, 4):
            validator = new CPShaderValidator14((DWORD*)code.data(), &caps, SHADER_VALIDATOR_LOG_ERRORS);
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

    return code;
}
//------------------------------------------------------------------------------
std::string ShaderAssemblerD3D8::Disassemble(std::vector<uint32_t> const& code)
{
    // Code width
    size_t count = 1;
    size_t width = 1;
    for (size_t i = 0; i < code.size(); ++i)
    {
        if (code[i] & 0x80000000)
        {
            count++;
            continue;
        }
        if (width < count)
        {
            width = count;
        }
        count = 1;
    }

    bool vs = true;
    bool ps14 = false;
    std::string text;
    for (size_t i = 0; i < code.size(); ++i)
    {
        char temp[256];
        snprintf(temp, 256, "%3zd: ", i);
        text += temp;

        // Opcode
        uint32_t opcode = code[i];

        // Code
        snprintf(temp, 256, "%08X ", opcode);
        text += temp;
        bool empty = false;
        for (size_t j = 1; j < width; ++j)
        {
            if (empty || (i + j) >= code.size() || (code[i + j] & 0x80000000) == 0)
            {
                empty = true;
                snprintf(temp, 256, "%8s ", "");
            }
            else
            {
                snprintf(temp, 256, "%08X ", code[i + j]);
            }
            text += temp;
        }

        // Version
        if (i == 0)
        {
            switch (opcode)
            {
            case D3DVS_VERSION(1, 0):
            case D3DVS_VERSION(1, 1):
                vs = true;
                ps14 = false;
                text += "vs.";
                break;
            case D3DPS_VERSION(1, 0):
            case D3DPS_VERSION(1, 1):
            case D3DPS_VERSION(1, 2):
            case D3DPS_VERSION(1, 3):
                vs = false;
                ps14 = false;
                text += "ps.";
                break;
            case D3DPS_VERSION(1, 4):
                vs = false;
                ps14 = true;
                text += "ps.";
                break;
            }
            text += std::to_string(D3DSHADER_VERSION_MAJOR(opcode));
            text += '.';
            text += std::to_string(D3DSHADER_VERSION_MINOR(opcode));
            text += '\n';
            continue;
        }

        // Instruction
        char const* instruction;
        switch (opcode & D3DSI_OPCODE_MASK)
        {
        case D3DSIO_NOP:            instruction = "nop";                    break;
        case D3DSIO_MOV:            instruction = "mov";                    break;
        case D3DSIO_ADD:            instruction = "add";                    break;
        case D3DSIO_SUB:            instruction = "sub";                    break;
        case D3DSIO_MAD:            instruction = "mad";                    break;
        case D3DSIO_MUL:            instruction = "mul";                    break;
        case D3DSIO_RCP:            instruction = "rcp";                    break;
        case D3DSIO_RSQ:            instruction = "rsq";                    break;
        case D3DSIO_DP3:            instruction = "dp3";                    break;
        case D3DSIO_DP4:            instruction = "dp4";                    break;
        case D3DSIO_MIN:            instruction = "min";                    break;
        case D3DSIO_MAX:            instruction = "max";                    break;
        case D3DSIO_SLT:            instruction = "slt";                    break;
        case D3DSIO_SGE:            instruction = "sge";                    break;
        case D3DSIO_EXP:            instruction = "exp";                    break;
        case D3DSIO_LOG:            instruction = "log";                    break;
        case D3DSIO_LIT:            instruction = "lit";                    break;
        case D3DSIO_DST:            instruction = "dst";                    break;
        case D3DSIO_LRP:            instruction = "lrp";                    break;
        case D3DSIO_FRC:            instruction = "frc";                    break;
        case D3DSIO_M4x4:           instruction = "m4x4";                   break;
        case D3DSIO_M4x3:           instruction = "m4x3";                   break;
        case D3DSIO_M3x4:           instruction = "m3x4";                   break;
        case D3DSIO_M3x3:           instruction = "m3x3";                   break;
        case D3DSIO_M3x2:           instruction = "m3x2";                   break;
#if (DIRECT3D_VERSION >= 0x0900)
        case D3DSIO_CALL:           instruction = "call";                   break;
        case D3DSIO_CALLNZ:         instruction = "callnz";                 break;
        case D3DSIO_LOOP:           instruction = "loop";                   break;
        case D3DSIO_RET:            instruction = "ret";                    break;
        case D3DSIO_ENDLOOP:        instruction = "endloop";                break;
        case D3DSIO_LABEL:          instruction = "label";                  break;
        case D3DSIO_DCL:            instruction = "dcl";                    break;
        case D3DSIO_POW:            instruction = "pow";                    break;
        case D3DSIO_CRS:            instruction = "crs";                    break;
        case D3DSIO_SGN:            instruction = "sgn";                    break;
        case D3DSIO_ABS:            instruction = "abs";                    break;
        case D3DSIO_NRM:            instruction = "nrm";                    break;
        case D3DSIO_SINCOS:         instruction = "sincos";                 break;
        case D3DSIO_REP:            instruction = "rep";                    break;
        case D3DSIO_ENDREP:         instruction = "endrep";                 break;
        case D3DSIO_IF:             instruction = "if";                     break;
        case D3DSIO_IFC:            instruction = "ifc";                    break;
        case D3DSIO_ELSE:           instruction = "else";                   break;
        case D3DSIO_ENDIF:          instruction = "endif";                  break;
        case D3DSIO_BREAK:          instruction = "break";                  break;
        case D3DSIO_BREAKC:         instruction = "breakc";                 break;
        case D3DSIO_MOVA:           instruction = "mova";                   break;
        case D3DSIO_DEFB:           instruction = "defb";                   break;
        case D3DSIO_DEFI:           instruction = "defi";                   break;
#endif
        case D3DSIO_TEXCOORD:       instruction = "texcoord";               break;
        case D3DSIO_TEXKILL:        instruction = "texkill";                break;
        case D3DSIO_TEX:            instruction = ps14 ? "texld" : "tex";   break;
        case D3DSIO_TEXBEM:         instruction = "texbem";                 break;
        case D3DSIO_TEXBEML:        instruction = "texbeml";                break;
        case D3DSIO_TEXREG2AR:      instruction = "texreg2ar";              break;
        case D3DSIO_TEXREG2GB:      instruction = "texreg2gb";              break;
        case D3DSIO_TEXM3x2PAD:     instruction = "texm3x2pad";             break;
        case D3DSIO_TEXM3x2TEX:     instruction = "texm3x2tex";             break;
        case D3DSIO_TEXM3x3PAD:     instruction = "texm3x3pad";             break;
        case D3DSIO_TEXM3x3TEX:     instruction = "texm3x3tex";             break;
        case D3DSIO_TEXM3x3DIFF:    instruction = "texm3x3diff";            break;
        case D3DSIO_TEXM3x3SPEC:    instruction = "texm3x3spec";            break;
        case D3DSIO_TEXM3x3VSPEC:   instruction = "texm3x3vspec";           break;
        case D3DSIO_EXPP:           instruction = "expp";                   break;
        case D3DSIO_LOGP:           instruction = "logp";                   break;
        case D3DSIO_CND:            instruction = "cnd";                    break;
        case D3DSIO_DEF:            instruction = "def";                    break;
        case D3DSIO_TEXREG2RGB:     instruction = "texreg2rgb";             break;
        case D3DSIO_TEXDP3TEX:      instruction = "texdp3tex";              break;
        case D3DSIO_TEXM3x2DEPTH:   instruction = "texm3x2depth";           break;
        case D3DSIO_TEXDP3:         instruction = "texdp3";                 break;
        case D3DSIO_TEXM3x3:        instruction = "texm3x3";                break;
        case D3DSIO_TEXDEPTH:       instruction = "texdepth";               break;
        case D3DSIO_CMP:            instruction = "cmp";                    break;
        case D3DSIO_BEM:            instruction = "bem";                    break;
#if (DIRECT3D_VERSION >= 0x0900)
        case D3DSIO_DP2ADD:         instruction = "dp2add";                 break;
        case D3DSIO_DSX:            instruction = "dsx";                    break;
        case D3DSIO_DSY:            instruction = "dsy";                    break;
        case D3DSIO_TEXLDD:         instruction = "texldd";                 break;
        case D3DSIO_SETP:           instruction = "setp";                   break;
        case D3DSIO_TEXLDL:         instruction = "texldl";                 break;
        case D3DSIO_BREAKP:         instruction = "breakp";                 break;
#endif
        case D3DSIO_PHASE:          instruction = "phase";                  break;
        case D3DSIO_END:            instruction = "end";                    break;
        default:                    instruction = "?";                      break;
        }
#if (DIRECT3D_VERSION >= 0x0900)
        if ((opcode & D3DSI_OPCODE_MASK) == D3DSIO_DCL && code.size() > (i + 1))
        {
            ++i;
            switch (code[i] & D3DSP_DCL_USAGE_MASK)
            {
            case D3DDECLUSAGE_POSITION:     instruction = "dcl_position";       break;
            case D3DDECLUSAGE_BLENDWEIGHT:  instruction = "dcl_blendweight";    break;
            case D3DDECLUSAGE_BLENDINDICES: instruction = "dcl_blendindices";   break;
            case D3DDECLUSAGE_NORMAL:       instruction = "dcl_normal";         break;
            case D3DDECLUSAGE_PSIZE:        instruction = "dcl_psize";          break;
            case D3DDECLUSAGE_TEXCOORD:     instruction = "dcl_texcoord";       break;
            case D3DDECLUSAGE_TANGENT:      instruction = "dcl_tangent";        break;
            case D3DDECLUSAGE_BINORMAL:     instruction = "dcl_binormal";       break;
            case D3DDECLUSAGE_TESSFACTOR:   instruction = "dcl_tessfactor";     break;
            case D3DDECLUSAGE_POSITIONT:    instruction = "dcl_positiont";      break;
            case D3DDECLUSAGE_COLOR:        instruction = "dcl_color";          break;
            case D3DDECLUSAGE_FOG:          instruction = "dcl_fog";            break;
            case D3DDECLUSAGE_DEPTH:        instruction = "dcl_depth";          break;
            case D3DDECLUSAGE_SAMPLE:       instruction = "dcl_sample";         break;
            }
        }
#endif
        char const* shift = "";
        char const* modifier = "";
        if ((opcode & D3DSI_OPCODE_MASK) != D3DSIO_DEF && code.size() > (i + 1))
        {
            switch (code[i + 1] & D3DSP_DSTSHIFT_MASK)
            {
            case 1  << D3DSP_DSTSHIFT_SHIFT:    shift = "_x2";  break;
            case 2  << D3DSP_DSTSHIFT_SHIFT:    shift = "_x4";  break;
            case 3  << D3DSP_DSTSHIFT_SHIFT:    shift = "_x8";  break;
            case 15 << D3DSP_DSTSHIFT_SHIFT:    shift = "_d2";  break;
            case 14 << D3DSP_DSTSHIFT_SHIFT:    shift = "_d4";  break;
            case 13 << D3DSP_DSTSHIFT_SHIFT:    shift = "_d8";  break;
            }
            switch (code[i + 1] & D3DSP_DSTMOD_MASK)
            {
            case D3DSPDM_SATURATE:  modifier = "_sat";  break;
            }
        }
        int length = 0;
        snprintf(temp, 256, "%12s", "");
        if (opcode & D3DSI_COISSUE)
        {
            length = snprintf(temp, 256, "%s%s%s%s", "+", instruction, shift, modifier);
        }
        else
        {
            length = snprintf(temp, 256, "%s%s%s%s", "", instruction, shift, modifier);
        }
        temp[length] = ' ';
        text += temp;

        // End
        if ((opcode & D3DSI_OPCODE_MASK) == D3DSIO_END)
        {
            text += '\n';
            break;
        }

        // Parameter
        for (size_t j = 0; j < 5; ++j)
        {
            ++i;
            if (code.size() <= i)
            {
                --i;
                break;
            }

            // Constant
            if (j != 0 && (opcode & D3DSI_OPCODE_MASK) == D3DSIO_DEF)
            {
                snprintf(temp, 256, ", %.1f", (float&)code[i]);
                text += temp;
                continue;
            }

            // Register
            if ((code[i] & 0x80000000) == 0)
            {
                --i;
                break;
            }
            text += (j == 0) ? " " : ", ";

            // Opcode
            uint32_t opcode = code[i];

            // Type
            char const* reg = nullptr;
            uint32_t type = 0;
            uint32_t index = 0;
#if (DIRECT3D_VERSION >= 0x0900)
            type |= (opcode & D3DSP_REGTYPE_MASK) >> D3DSP_REGTYPE_SHIFT;
            type |= (opcode & D3DSP_REGTYPE_MASK2) >> D3DSP_REGTYPE_SHIFT2;
#else
            type |= (opcode & D3DSP_REGTYPE_MASK);
#endif
            index |= (opcode & D3DSP_REGNUM_MASK);
            switch (type)
            {
            case D3DSPR_TEMP:           reg = "r";              break;
            case D3DSPR_INPUT:          reg = "v";              break;
            case D3DSPR_CONST:          reg = "c";              break;
//          case D3DSPR_TEXTURE:
            case D3DSPR_ADDR:           reg = vs ? "a" : "t";   break;
            case D3DSPR_RASTOUT:
                switch (index)
                {
                case D3DSRO_POSITION:   reg = "oPos";           break;
                case D3DSRO_FOG:        reg = "oFog";           break;
                case D3DSRO_POINT_SIZE: reg = "oPts";           break;
                }
                break;
            case D3DSPR_ATTROUT:        reg = "oD";             break;
//          case D3DSPR_OUTPUT:
            case D3DSPR_TEXCRDOUT:      reg = "oT";             break;
#if (DIRECT3D_VERSION >= 0x0900)
            case D3DSPR_CONSTINT:       reg = "c";              break;
            case D3DSPR_COLOROUT:       reg = "oC";             break;
            case D3DSPR_DEPTHOUT:       reg = "oD";             break;
            case D3DSPR_SAMPLER:        reg = "s";              break;
            case D3DSPR_CONST2:         reg = "c";              break;
            case D3DSPR_CONST3:         reg = "c";              break;
            case D3DSPR_CONST4:         reg = "c";              break;
            case D3DSPR_CONSTBOOL:      reg = "c";              break;
            case D3DSPR_LOOP:           reg = "?";              break;
            case D3DSPR_TEMPFLOAT16:    reg = "?";              break;
            case D3DSPR_MISCTYPE:       reg = "?";              break;
            case D3DSPR_LABEL:          reg = "?";              break;
            case D3DSPR_PREDICATE:      reg = "?";              break;
#endif
            default:                    reg = "?";              break;
            }

            // Source / Destination
            if (j != 0)
            {
                switch (opcode & D3DSP_SRCMOD_MASK)
                {
                case D3DSPSM_NEG:
                case D3DSPSM_BIASNEG:
                case D3DSPSM_SIGNNEG:
                case D3DSPSM_X2NEG:     text += "-";            break;
                case D3DSPSM_COMP:      text += "1-";           break;
                }
                text += reg;
                if (type != D3DSPR_RASTOUT)
                {
                    text += std::to_string(index);
                }
                switch (opcode & D3DSP_SRCMOD_MASK)
                {
                case D3DSPSM_BIAS:
                case D3DSPSM_BIASNEG:   text += "_bias";        break;
                case D3DSPSM_SIGN:
                case D3DSPSM_SIGNNEG:   text += "_bx2";         break;
                case D3DSPSM_X2:
                case D3DSPSM_X2NEG:     text += "_x2";          break;
                case D3DSPSM_DZ:        text += "_dz";          break;
                case D3DSPSM_DW:        text += "_dw";          break;
                }
            }
            else
            {
                text += reg;
                if (type != D3DSPR_RASTOUT)
                {
                    text += std::to_string(index);
                }
            }

            // Swizzle / WriteMask
            if (j != 0 && (opcode & D3DSP_SWIZZLE_MASK) != D3DSP_NOSWIZZLE)
            {
                text += '.';
                for (size_t i = 0; i < 4; ++i)
                {
                    switch ((opcode >> (D3DSP_SWIZZLE_SHIFT + i * 2)) & 0x3)
                    {
                    case 0: text += "x";    break;
                    case 1: text += "y";    break;
                    case 2: text += "z";    break;
                    case 3: text += "w";    break;
                    }
                }
            }
            else if (j == 0 && (opcode & D3DSP_WRITEMASK_ALL) != D3DSP_WRITEMASK_ALL)
            {
                text += '.';
                text += (opcode & D3DSP_WRITEMASK_0) == D3DSP_WRITEMASK_0 ? "x" : "";
                text += (opcode & D3DSP_WRITEMASK_1) == D3DSP_WRITEMASK_1 ? "y" : "";
                text += (opcode & D3DSP_WRITEMASK_2) == D3DSP_WRITEMASK_2 ? "z" : "";
                text += (opcode & D3DSP_WRITEMASK_3) == D3DSP_WRITEMASK_3 ? "w" : "";
            }
        }

        text += '\n';
    }

    return text;
}
//==============================================================================
