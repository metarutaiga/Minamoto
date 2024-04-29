//==============================================================================
// Minamoto : Tools Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <vector>
#include <utility/xxCamera.h>
#include <utility/xxMath.h>
#include "Tools.h"

//==============================================================================
static std::vector<std::pair<xxVector3, xxVector3>> lines;
static std::vector<std::pair<xxVector3, float>> spheres;
//------------------------------------------------------------------------------
void Tools::Line(xxVector3 const& from, xxVector3 const& to)
{
    lines.push_back({from, to});
}
//------------------------------------------------------------------------------
void Tools::Sphere(xxVector3 const& point, float scale)
{
    spheres.push_back({point, scale});
}
//------------------------------------------------------------------------------
void Tools::Draw(xxCameraPtr const& camera)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGuiViewport* viewport = ImGui::GetWindowViewport();

    // Line
    for (auto const& line : lines)
    {
        xxVector2 from = camera->GetWorldPosToScreenPos(line.first).xy;
        xxVector2 to = camera->GetWorldPosToScreenPos(line.second).xy;
        from = from * xxVector2{viewport->Size.x, viewport->Size.y} + xxVector2{viewport->Pos.x, viewport->Pos.y};
        to = to * xxVector2{viewport->Size.x, viewport->Size.y} + xxVector2{viewport->Pos.x, viewport->Pos.y};
        drawList->AddLine(ImVec2(from.x, from.y), ImVec2(to.x, to.y), 0xFFFFFFFF);
    }
    lines.clear();

    // Sphere
    for (auto const& sphere : spheres)
    {
        xxVector2 center = camera->GetWorldPosToScreenPos(sphere.first).xy;
        float radius = 1.0f;
        if (sphere.second != 0.0f)
        {
            radius = (camera->GetWorldPosToScreenPos(sphere.first + camera->Right * sphere.second).xy - center).x;
            radius = radius * std::max(viewport->Size.x, viewport->Size.y);
        }
        center = center * xxVector2{viewport->Size.x, viewport->Size.y} + xxVector2{viewport->Pos.x, viewport->Pos.y};
        drawList->AddCircle(ImVec2(center.x, center.y), radius, 0xFFFFFFFF);
    }
    spheres.clear();
}
//------------------------------------------------------------------------------
void Tools::LookAtFromBound(xxCameraPtr const& camera, xxVector4 bound, xxVector3 const& up)
{
    bound.x = std::roundf(bound.x);

    camera->Location.x = bound.x;
    camera->Location.y = bound.w * -2.0f;
    camera->Location.z = bound.z;
    camera->LookAt(bound.xyz, up);
}
//==============================================================================
