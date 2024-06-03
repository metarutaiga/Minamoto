//==============================================================================
// Minamoto : Tools Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <vector>
#include <xxGraphicPlus/xxCamera.h>
#include <xxGraphicPlus/xxMath.h>
#include <Tools/CameraTools.h>
#include "Tools.h"

//==============================================================================
struct DrawLine { xxVector3 from; xxVector3 to; ImU32 col; };
struct DrawSphere { xxVector3 point; float scale; ImU32 col; };
struct DrawRect { xxVector2 leftTop; xxVector2 rightBottom; ImU32 col; };
//------------------------------------------------------------------------------
static std::vector<DrawLine> lines;
static std::vector<DrawSphere> spheres;
static std::vector<DrawRect> rects;
//------------------------------------------------------------------------------
void Tools::Line(xxVector3 const& from, xxVector3 const& to, ImU32 col)
{
    lines.push_back({from, to, col});
}
//------------------------------------------------------------------------------
void Tools::Sphere(xxVector3 const& point, float scale, ImU32 col)
{
    spheres.push_back({point, scale, col});
}
//------------------------------------------------------------------------------
void Tools::Rect(xxVector2 const& leftTop, xxVector2 const& rightBottom, ImU32 col)
{
    rects.push_back({leftTop, rightBottom, col});
}
//------------------------------------------------------------------------------
void Tools::Draw(xxCameraPtr const& camera, xxVector2 const& scale, xxVector2 const& offset)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Line
    for (auto const& line : lines)
    {
        xxVector4 from = CameraTools::GetWorldPosToScreenPos(camera, line.from);
        xxVector4 to = CameraTools::GetWorldPosToScreenPos(camera, line.to);
        if (from.w < 0.0f || to.w < 0.0f)
            continue;
        from.xy = from.xy * scale + offset;
        to.xy = to.xy * scale + offset;
        drawList->AddLine(ImVec2(from.x, from.y), ImVec2(to.x, to.y), line.col);
    }
    lines.clear();

    // Sphere
    for (auto const& sphere : spheres)
    {
        xxVector4 center = CameraTools::GetWorldPosToScreenPos(camera, sphere.point);
        if (center.w < 0.0f)
            continue;
        float radius = 1.0f;
        if (sphere.scale != 0.0f)
        {
            xxVector2 radius2 = (CameraTools::GetWorldPosToScreenPos(camera, sphere.point + camera->Right * sphere.scale).xy - center.xy) * scale;
            radius = std::max(radius2.x, radius2.y);
        }
        center.xy = center.xy * scale + offset;
        drawList->AddCircle(ImVec2(center.x, center.y), radius, sphere.col);
    }
    spheres.clear();

    // Rect
    for (auto const& rect : rects)
    {
        xxVector2 leftTop = rect.leftTop * scale + offset;
        xxVector2 rightBottom = rect.rightBottom * scale + offset;
        drawList->AddRect(ImVec2(leftTop.x, leftTop.y), ImVec2(rightBottom.x, rightBottom.y), rect.col);
    }
    rects.clear();
}
//------------------------------------------------------------------------------
void Tools::LookAtFromBound(xxCameraPtr const& camera, xxVector4 bound, xxVector3 const& up)
{
    if (bound.w == 0.0f)
    {
        camera->Location = (xxVector3::Y * -10 + xxVector3::Z * 10);
        camera->LookAt(xxVector3::ZERO, up);
        return;
    }
    bound.x = std::roundf(bound.x);

    camera->Location.x = bound.x;
    camera->Location.y = bound.w * -2.0f;
    camera->Location.z = bound.z;
    camera->LookAt(bound.xyz, up);
}
//==============================================================================
