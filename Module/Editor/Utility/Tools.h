//==============================================================================
// Minamoto : Tools Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct Tools
{
    static void Line(xxVector3 const& from, xxVector3 const& to);
    static void Sphere(xxVector3 const& point, float scale);
    static void Draw(xxCameraPtr const& camera);
};
