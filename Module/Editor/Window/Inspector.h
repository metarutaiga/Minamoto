//==============================================================================
// Minamoto : Inspector Header
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

class Inspector
{
    static xxNodePtr selected;
public:
    static void Initialize();
    static void Shutdown();
    static void Select(xxNodePtr const& node);
    static bool Update(const UpdateData& updateData, bool& show, xxCameraPtr const& camera);
};
