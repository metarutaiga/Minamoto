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
#if defined(__clang__)
#include <quickjs/repl.c>
#endif
}

static Console console;
//------------------------------------------------------------------------------
void QuickJSConsole::Initialize()
{
    QuickJS::StandardLibrary();
#if defined(__clang__)
    QuickJS::Eval(qjsc_repl, qjsc_repl_size);
#endif
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
        if (ImGui::IsWindowFocused())
        {
            update |= console.UpdateInput(updateData);
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
                console.AddHistory(input.c_str());
                input.clear();
                inputPos = 0;
            }
            QuickJS::Update();
            update = true;

            if (appearing && input.empty() == false)
                std::swap(input[0], c);
        }

        auto const& lines = QuickJS::Outputs;
        update |= console.UpdateConsole(updateData, lines);
    }
    ImGui::End();

    return update;
}
//------------------------------------------------------------------------------
