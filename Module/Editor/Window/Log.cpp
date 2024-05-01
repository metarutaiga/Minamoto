//==============================================================================
// Minamoto : Log Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include <queue>
#include "Log.h"

std::deque<char*> Log::systemLog;
float Log::windowHeights[2];
//------------------------------------------------------------------------------
void Log::Initialize()
{
    
}
//------------------------------------------------------------------------------
void Log::Shutdown()
{
    for (char* line : systemLog)
        xxFree(line);
    systemLog = std::deque<char*>();
}
//------------------------------------------------------------------------------
bool Log::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    if (ImGui::Begin(ICON_FA_DESKTOP "Log", &show))
    {
        updateData.message({ "LOGGER_UPDATE", (char*)&systemLog });

        ImGuiListClipper clipper;
        clipper.Begin((int)systemLog.size(), ImGui::GetTextLineHeightWithSpacing());
        while (clipper.Step())
        {
            auto start = systemLog.begin() + clipper.DisplayStart;
            auto end = systemLog.begin() + clipper.DisplayEnd;
            for (auto it = start; it != end; ++it)
            {
                ImGui::Selectable(*it, false);
            }
        }
        clipper.End();

        static size_t logCount = 0;
        if (logCount != systemLog.size())
        {
            logCount = systemLog.size();
            ImGui::TextUnformatted("");
            ImGui::SetScrollHereY();
        }

        ImGui::End();
    }

    return false;
}
//------------------------------------------------------------------------------
float Log::GetWindowHeight()
{
    return windowHeights[1];
}
//------------------------------------------------------------------------------
