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
void Console::Update(const UpdateData& updateData)
{
    ImGuiIO& io = ImGui::GetIO();
    for (int i = 0; i < io.InputQueueCharacters.size(); ++i)
    {
        ImWchar c = io.InputQueueCharacters[i];
        switch (c)
        {
        case '\b':
        case 0x7F:
            if (input.empty() == false && inputPos && input.size() >= inputPos)
            {
                input.erase(input.begin() + inputPos - 1, input.begin() + inputPos);
                inputPos--;
            }
            continue;
        case 0xF700:
            if (historyPos)
                historyPos--;
            input = history[historyPos];
            inputPos = input.size();
            continue;
        case 0xF701:
            if (historyPos < history.size())
                historyPos++;
            input = history.size() > historyPos ? history[historyPos] : "";
            inputPos = input.size();
            continue;
        case 0xF702:
            if (inputPos)
                inputPos--;
            continue;
        case 0xF703:
            if (inputPos < input.size())
                inputPos++;
            continue;
        case 0xF728:
            if (input.empty() == false && input.size() > inputPos)
                input.erase(input.begin() + inputPos, input.begin() + inputPos + 1);
            continue;
        case 0xF729:
            inputPos = 0;
            continue;
        case 0xF72B:
            inputPos = input.size();
            continue;
        }
        input.insert(inputPos, 1, char(c));
        inputPos++;
    }
    io.InputQueueCharacters.clear();
    if (input.empty() == false && input[0] == 0)
    {
        input.clear();
        inputPos = 0;
    }
}
//------------------------------------------------------------------------------
void Console::AddHistory(char const* line)
{
    if (line == nullptr || line[0] == 0 || line[0] == '\r' || line[0] == '\n')
        return;
    history.push_back(line);
    historyPos = history.size();
}
//------------------------------------------------------------------------------
