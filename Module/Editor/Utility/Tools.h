//==============================================================================
// Minamoto : Tools Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct Tools
{
    static void Line(xxVector3 const& from, xxVector3 const& to, ImU32 col = 0xFFFFFFFF);
    static void Sphere(xxVector3 const& point, float scale, ImU32 col = 0xFFFFFFFF);
    static void Rect(xxVector2 const& leftTop, xxVector2 const& rightBottom, ImU32 col = 0xFFFFFFFF);
    static void Draw(xxCameraPtr const& camera, xxVector2 const& scale, xxVector2 const& offset);
    static void LookAtFromBound(xxCameraPtr const& camera, xxVector4 bound, xxVector3 const& up);
};
