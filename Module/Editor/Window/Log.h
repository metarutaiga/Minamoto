//==============================================================================
// Minamoto : Log Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct Log
{
    static void Initialize();
    static void Shutdown();
    static bool Update(const UpdateData& updateData, bool& show);
};
