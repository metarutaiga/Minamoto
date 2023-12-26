//==============================================================================
// Minamoto : Log Source
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <queue>
#include "Log.h"

std::deque<char*> Log::systemLog;
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

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    int flags;

    float titleHeight = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;
    float borderHeight = style.FramePadding.y * 2.0f;
    float windowHeight = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;
    if (collapse)
    {
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y - 1.0f + viewport->Size.y - titleHeight - borderHeight - windowHeight * 10.0f));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, titleHeight + borderHeight + windowHeight * 10.0f));
        flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysVerticalScrollbar;
    }
    else
    {
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y - 1.0f + viewport->Size.y - borderHeight * 2.0f - windowHeight));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, borderHeight * 2.0f + windowHeight));
        flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
    }

    if (ImGui::Begin("Log", nullptr, flags))
    {
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
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
