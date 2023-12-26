//==============================================================================
// Minamoto : Hierarchy Header
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

namespace IGFD { class FileDialog; }

class Hierarchy
{
    static xxNodePtr selectedLeft;
    static xxNodePtr selectedRight;
    static xxNodePtr importNode;
    static IGFD::FileDialog* fileDialog;
public:
    static void Initialize();
    static void Shutdown();
    static bool Update(const UpdateData& updateData, bool& show, xxNodePtr const& root);
};
