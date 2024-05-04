//==============================================================================
// Minamoto : Console Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <queue>
#include "Console.h"

//------------------------------------------------------------------------------
bool Console::Update(const UpdateData& updateData)
{
    bool update = false;
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < io.InputQueueCharacters.size(); ++i)
    {
        ImWchar c = io.InputQueueCharacters[i];
        switch (c)
        {
        case '\b':
        case 0x7F:
            if (input.empty() == false && input.size() >= inputPos && inputPos >= 1)
            {
                input.erase(input.begin() + inputPos - 1, input.begin() + inputPos);
                inputPos--;
            }
            break;
        case 0xF700:
            if (historyPos)
                historyPos--;
            input = history[historyPos];
            inputPos = input.size();
            break;
        case 0xF701:
            if (historyPos < history.size())
                historyPos++;
            input = history.size() > historyPos ? history[historyPos] : std::string();
            inputPos = input.size();
            break;
        case 0xF702:
            if (inputPos)
                inputPos--;
            break;
        case 0xF703:
            if (inputPos < input.size())
                inputPos++;
            break;
        case 0xF728:
            if (input.empty() == false && input.size() > inputPos)
                input.erase(input.begin() + inputPos, input.begin() + inputPos + 1);
            break;
        case 0xF729:
            inputPos = 0;
            break;
        case 0xF72B:
            inputPos = input.size();
            break;
        default:
            input.insert(inputPos, 1, char(c));
            inputPos++;
            break;
        }
        update = true;
    }
    io.InputQueueCharacters.clear();
    if (input.empty() == false && input[0] == 0)
    {
        input.clear();
        inputPos = 0;
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
    if (history.empty() == false && history.back() == temp)
        return;
    history.push_back(temp);
    historyPos = history.size();
}
//------------------------------------------------------------------------------
