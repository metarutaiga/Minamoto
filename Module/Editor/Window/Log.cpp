//==============================================================================
// Minamoto : Log Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <queue>
#include "Log.h"

static std::deque<char*> systemLog;
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

        if (systemLog.empty() == false)
        {
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
        }

        static size_t logCount = 0;
        if (logCount != systemLog.size())
        {
            logCount = systemLog.size();
            ImGui::TextUnformatted("");
            ImGui::SetScrollHereY();
        }
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
