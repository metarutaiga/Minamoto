//==============================================================================
// Minamoto : Game Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

class Game
{
public:
    static void Initialize();
    static void Shutdown(bool suspend = false);
    static bool Update(const UpdateData& updateData, bool& show);
    static void Callback(const ImDrawList* list, const ImDrawCmd* cmd);
};
