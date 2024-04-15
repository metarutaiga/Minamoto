//==============================================================================
// Minamoto : Log Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
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

    static bool collapse = false;
    static bool resetScroll = false;
    static size_t logCount = 0;

    int flags;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (collapse)
    {
        flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar;
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - windowHeights[0]));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, windowHeights[0]));
    }
    else
    {
        flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - windowHeights[1]));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, windowHeights[1]));
    }

    if (ImGui::Begin("Log", nullptr, flags))
    {
        if (collapse)
        {
            windowHeights[0] = windowHeights[1] * 5.0f;
        }
        else
        {
            windowHeights[1] = ImGui::GetWindowHeight();
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            collapse = !collapse;
            resetScroll = true;
        }

        updateData.message({ "LOGGER_UPDATE", (char*)&systemLog });

        if (collapse)
        {
            ImGuiListClipper clipper;
            clipper.Begin((int)systemLog.size(), ImGui::GetTextLineHeightWithSpacing());
            while (clipper.Step())
            {
                auto start = systemLog.begin() + clipper.DisplayStart;
                auto end = systemLog.begin() + clipper.DisplayEnd;
                for (auto it = start; it != end; ++it)
                {
                    ImGui::TextUnformatted(*it);
                }
            }
            clipper.End();

            if (resetScroll || logCount != systemLog.size())
            {
                resetScroll = false;
                logCount = systemLog.size();
                ImGui::TextUnformatted("");
                ImGui::SetScrollHereY();
            }
        }
        else if (systemLog.size())
        {
            ImGui::TextUnformatted(systemLog.back());
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
