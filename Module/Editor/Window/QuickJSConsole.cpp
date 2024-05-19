//==============================================================================
// Minamoto : QuickJSConsole Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <queue>
#include "Script/QuickJS.h"
#include "Component/Console.h"
#include "QuickJSConsole.h"

extern "C"
{
#include <quickjs/repl.c>
}

static Console console;
//------------------------------------------------------------------------------
void QuickJSConsole::Initialize()
{
    QuickJS::Eval(qjsc_repl, qjsc_repl_size);
}
//------------------------------------------------------------------------------
void QuickJSConsole::Shutdown()
{
}
//------------------------------------------------------------------------------
bool QuickJSConsole::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    bool update = false;
    if (ImGui::Begin(ICON_FA_TABLET "QuickJS Console", &show))
    {
        if (ImGui::IsWindowHovered())
        {
            update |= console.Update(updateData);
        }

        bool appearing = ImGui::IsWindowAppearing();
        if (appearing || ImGui::IsKeyReleased(ImGuiKey_Enter))
        {
            auto& input = console.input;
            auto& inputPos = console.inputPos;

            char c = 0;
            if (appearing && input.empty() == false)
                std::swap(input[0], c);

            if (input.empty() == false && input[0] != 0)
            {
                for (char c : input)
                {
                    QuickJS::Input(c);
                }
                QuickJS::Input('\n');
                input.clear();
                inputPos = 0;
            }
            QuickJS::Update();

            if (appearing && input.empty() == false)
                std::swap(input[0], c);
        }

        auto const& lines = QuickJS::Outputs;
        if (lines.empty() == false)
        {
            ImGuiListClipper clipper;
            clipper.Begin((int)lines.size(), ImGui::GetTextLineHeightWithSpacing());
            while (clipper.Step())
            {
                auto start = lines.begin() + clipper.DisplayStart;
                auto end = lines.begin() + clipper.DisplayEnd;
                for (auto it = start; it != end; ++it)
                {
                    if (it == end - 1)
                    {
                        static int blink = 0;
                        if (blink != (int(updateData.time * 2.0f) & 1))
                        {
                            blink = (int(updateData.time * 2.0f) & 1);
                            update = true;
                        }

                        auto& input = console.input;
                        auto inputPos = console.inputPos;
                        static std::string temp;
                        temp = (*it);
                        temp.append(input.c_str(), inputPos);
                        temp.append(blink ? "|" : " ");
                        if (input.size() > inputPos)
                            temp.append(input.c_str() + inputPos, input.size() - inputPos);
                        ImGui::Selectable(temp.c_str(), false);
                        continue;
                    }
                    ImGui::Selectable((*it).c_str(), false);
                }
            }
        }

        static size_t logCount = 0;
        if (logCount != lines.size())
        {
            logCount = lines.size();
            ImGui::TextUnformatted("");
            ImGui::SetScrollHereY();
        }
    }
    ImGui::End();

    return update;
}
//------------------------------------------------------------------------------
