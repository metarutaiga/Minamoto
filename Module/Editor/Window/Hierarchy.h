//==============================================================================
// Minamoto : Hierarchy Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

namespace IGFD { class FileDialog; }

class Hierarchy
{
    static float windowWidth;
    static xxNodePtr selectedLeft;
    static xxNodePtr selectedRight;
    static xxNodePtr importNode;
    static xxNodePtr exportNode;
    static char importName[];
    static char exportName[];
    static IGFD::FileDialog* importFileDialog;
    static IGFD::FileDialog* exportFileDialog;
    static bool drawNodeLine;
    static bool drawNodeBound;
public:
    static void Initialize();
    static void Shutdown();
    static void Import(const UpdateData& updateData, xxCameraPtr const& camera);
    static void Export(const UpdateData& updateData);
    static void Option(const UpdateData& updateData, float menuBarHeight, xxNodePtr const& root, xxCameraPtr const& camera);
    static bool Update(const UpdateData& updateData, float menuBarHeight, bool& show, xxNodePtr const& root, xxCameraPtr const& camera);
};
