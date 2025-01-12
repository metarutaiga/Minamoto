//==============================================================================
// Minamoto : DrawTools Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct RuntimeAPI DrawTools
{
    struct DrawData : public xxDrawData
    {
        xxCameraPtr     camera2D;
        xxCameraPtr     camera3D;
    };

    static void Draw(DrawData& drawData, xxNodePtr const& node);
protected:
    static void DrawTraversal(DrawData& drawData, xxNodePtr const& node);
};
