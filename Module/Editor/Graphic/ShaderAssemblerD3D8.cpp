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

#include <xxShaderAssembler/microsoft/errlog.hpp>
#include <xxShaderAssembler/microsoft/valbase.hpp>
#include <xxShaderAssembler/microsoft/pshdrval.hpp>
#include <xxShaderAssembler/microsoft/vshdrval.hpp>
#include <xxShaderAssembler/microsoft/pshader.h>
#include <xxShaderAssembler/microsoft/vshader.h>

#include <xxShaderAssembler/disasm.h>

extern "C" void ShaderOutputDebugString(char const* string) {}

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

    std::vector<uint32_t> tokens;
    auto reg = [&tokens](std::string_view view, bool source, int shift, bool saturate)
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
        tokens.push_back(opcode);
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
        tokens.push_back(opcode);
        if (dcl != UINT32_MAX)
        {
            tokens.push_back(dcl | 0x80000000);
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
                tokens.push_back((uint32_t&)value);
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
    tokens.push_back(D3DSIO_END);

    // Validator
    if (tokens.empty() == false && message.empty())
    {
        D3DCAPS8 caps = {};
        caps.VertexShaderVersion = D3DVS_VERSION(1, 1);
        caps.MaxVertexShaderConst = 96;
        caps.PixelShaderVersion = D3DPS_VERSION(1, 4);
        caps.MaxPixelShaderValue = 8.0f;

        CBaseShaderValidator* validator = nullptr;
        switch (tokens[0])
        {
        case D3DVS_VERSION(1, 0):
        case D3DVS_VERSION(1, 1):
            validator = new CVShaderValidator((DWORD*)tokens.data(), nullptr, &caps, SHADER_VALIDATOR_LOG_ERRORS);
            break;
        case D3DPS_VERSION(1, 0):
        case D3DPS_VERSION(1, 1):
        case D3DPS_VERSION(1, 2):
        case D3DPS_VERSION(1, 3):
            validator = new CPShaderValidator10((DWORD*)tokens.data(), &caps, SHADER_VALIDATOR_LOG_ERRORS);
            break;
        case D3DPS_VERSION(1, 4):
            validator = new CPShaderValidator14((DWORD*)tokens.data(), &caps, SHADER_VALIDATOR_LOG_ERRORS);
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

    return tokens;
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
