//==============================================================================
// Minamoto : ShaderAssembler Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include "ShaderAssemblerD3D8.h"
#include "ShaderAssemblerNV20.h"
#include "ShaderAssemblerR200.h"
#include "ShaderAssembler.h"

static std::string input;
static std::string output;
static std::string message;
//==============================================================================
void ShaderAssembler::Initialize()
{
}
//------------------------------------------------------------------------------
void ShaderAssembler::Shutdown()
{
    input = std::string();
    output = std::string();
    message = std::string();
}
//------------------------------------------------------------------------------
static char const vs_1_0[] =
R"(vs_1_0
mul r0, c0, v0.x
mad r0, c1, v0.y, r0
mad r0, c2, v0.z, r0
add r0, c3, r0
mul r1, c4, r0.x
mad r1, c5, r0.y, r1
mad r1, c6, r0.z, r1
mad r1, c7, r0.w, r1
mul r0, c8, r1.x
mad r0, c9, r1.y, r0
mad r0, c10, r1.z, r0
mad oPos, c11, r1.w, r0
mov oD0, v5
mov oT0.xy, v7)";
//------------------------------------------------------------------------------
static char const ps_1_0[] =
R"(ps_1_0
tex t0
mul r0, t0, v0)";
//------------------------------------------------------------------------------
static char const ps_1_4[] =
R"(ps_1_4
texld r0, t0
mul r0, r0, v0)";
//------------------------------------------------------------------------------
bool ShaderAssembler::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    ImGui::SetNextWindowSize(ImVec2(1280.0f, 720.0f), ImGuiCond_Appearing);
    if (ImGui::Begin(ICON_FA_PENCIL "Shader Assembler", &show, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking))
    {
        float labelWidth = ImGui::CalcTextSize("A").x * 8.0f;
        ImVec2 avail = ImGui::GetContentRegionAvail();
        avail.y -= ImGui::GetTextLineHeight() + 16.0f;

        ImGui::Columns(2);

        static int mode = "D3D8"_cc;
        bool compile = false;
        char const* example = nullptr;
        if (ImGui::Button("vs 1.0"))    example = vs_1_0;
        ImGui::SameLine();
        if (ImGui::Button("ps 1.0"))    example = ps_1_0;
        ImGui::SameLine();
        if (ImGui::Button("ps 1.4"))    example = ps_1_4;
        if (example)
        {
            input = example;
            compile = true;
        }
        compile |= ImGui::InputTextMultiline("INPUT", input, ImVec2(avail.x / 3.0f - labelWidth, avail.y / 2.0f), ImGuiInputTextFlags_AllowTabInput);

        ImGui::NextColumn();
        ImGui::SetColumnOffset(1, avail.x / 3.0f);

        compile |= ImGui::RadioButton("D3D8", &mode, "D3D8"_cc);
        ImGui::SameLine();
        compile |= ImGui::RadioButton("NV20", &mode, "NV20"_cc);
        ImGui::SameLine();
        compile |= ImGui::RadioButton("R200", &mode, "R200"_cc);
        ImGui::InputTextMultiline("OUTPUT", output, ImVec2((avail.x - avail.x / 3.0f) - labelWidth, avail.y / 2.0f), ImGuiInputTextFlags_ReadOnly);

        ImGui::Columns(1);

        ImGui::InputTextMultiline("MESSAGE", message, ImVec2(avail.x - labelWidth, avail.y / 2.0f), ImGuiInputTextFlags_ReadOnly);

        if (compile)
        {
            output.clear();
            message.clear();
            switch (mode)
            {
            case "D3D8"_cc:
                output = ShaderAssemblerD3D8::Disassemble(ShaderAssemblerD3D8::Assemble(input, message));
                break;
            case "NV20"_cc:
            {
                auto shader = ShaderAssemblerD3D8::Assemble(input, message);
                if (shader.empty() == false)
                {
                    switch (shader.front() & 0xFFFF0000)
                    {
                    case 0xFFFE0000:
                        output = ShaderAssemblerNV20::DisassembleCheops(ShaderAssemblerNV20::CompileCheops(shader, message));
                        break;
                    case 0xFFFF0000:
                        output = ShaderAssemblerNV20::DisassembleKelvin(ShaderAssemblerNV20::CompileKelvin(shader, message));
                        break;
                    }
                }
                break;
            }
            case "R200"_cc:
            {
                auto shader = ShaderAssemblerD3D8::Assemble(input, message);
                if (shader.empty() == false)
                {
                    switch (shader.front() & 0xFFFF0000)
                    {
                    case 0xFFFE0000:
                        output = ShaderAssemblerR200::DisassembleChaplinPVS(ShaderAssemblerR200::CompileChaplinPVS(shader, message));
                        break;
                    case 0xFFFF0000:
                        output = ShaderAssemblerR200::DisassembleChaplin(ShaderAssemblerR200::CompileChaplin(shader, message));
                        break;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
    ImGui::End();

    return false;
}
//==============================================================================
