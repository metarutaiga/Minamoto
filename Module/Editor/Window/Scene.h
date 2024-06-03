//==============================================================================
// Minamoto : Scene Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

class Scene
{
public:
    static xxCameraPtr screenCamera;
    static xxCameraPtr sceneCamera;
    static xxNodePtr sceneRoot;
public:
    static void Initialize();
    static void Shutdown(bool suspend = false);
    static void DrawBoneLine(xxNodePtr const& root);
    static void DrawNodeLine(xxNodePtr const& root);
    static void DrawNodeBound(xxNodePtr const& root);
    static bool Update(const UpdateData& updateData, bool& show);
    static void Callback(const ImDrawList* list, const ImDrawCmd* cmd);
};
