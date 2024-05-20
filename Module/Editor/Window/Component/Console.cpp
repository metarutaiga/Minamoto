//==============================================================================
// Minamoto : Console Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <queue>
#include "Console.h"

//------------------------------------------------------------------------------
bool Console::UpdateInput(const UpdateData& updateData)
{
    bool update = false;

    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < io.InputQueueCharacters.size(); ++i)
    {
        ImWchar c = io.InputQueueCharacters[i];
        input.insert(inputPos++, 1, char(c));
        update = true;
    }
    io.InputQueueCharacters.clear();

    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
    {
        if (inputPos)
        {
            inputPos--;
            update = true;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
    {
        if (inputPos < input.size())
        {
            inputPos++;
            update = true;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
    {
        if (historyPos)
        {
            historyPos--;
            input = history[historyPos];
            inputPos = input.size();
            update = true;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        if (historyPos < history.size())
        {
            historyPos++;
            input = history.size() > historyPos ? history[historyPos] : std::string();
            inputPos = input.size();
            update = true;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Home))
    {
        if (inputPos != 0)
        {
            inputPos = 0;
            update = true;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_End))
    {
        if (inputPos != input.size())
        {
            inputPos = input.size();
            update = true;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        if (input.empty() == false && input.size() > inputPos)
        {
            input.erase(input.begin() + inputPos, input.begin() + inputPos + 1);
            update = true;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace))
    {
        if (input.empty() == false && input.size() >= inputPos && inputPos >= 1)
        {
            input.erase(input.begin() + inputPos - 1, input.begin() + inputPos);
            inputPos--;
            update = true;
        }
    }

    if (input.empty() == false && input[0] == 0)
    {
        input.clear();
        inputPos = 0;
    }
    return update;
}
//------------------------------------------------------------------------------
bool Console::UpdateConsole(const UpdateData& updateData, std::deque<std::string> const& lines)
{
    bool update = false;
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
    if (lineCount != lines.size())
    {
        lineCount = lines.size();
        ImGui::TextUnformatted("");
        ImGui::SetScrollHereY();
        update = true;
    }
    return update;
}
//------------------------------------------------------------------------------
void Console::AddHistory(char const* line)
{
    std::string temp = line ? line : std::string();
    while (temp.empty() == false)
    {
        char c = temp.back();
        switch (c)
        {
        case 0:
            return;
        case '\r':
        case '\n':
            temp.pop_back();
            continue;
        }
        break;
    }
    if (temp.empty())
        return;
    if (history.empty() || history.back() != temp)
        history.push_back(temp);
    historyPos = history.size();
}
//------------------------------------------------------------------------------
