//==============================================================================
// Minamoto : Setup Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct Setup
{
    static void Initialize();
    static void Shutdown();
    static void Load();
    static void Save();
    static bool Update(const UpdateData& updateData, bool& show);
};
