//==============================================================================
// Minamoto : QuickJSConsole Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

class QuickJSConsole
{
public:
    static void Initialize();
    static void Shutdown();
    static bool Update(const UpdateData& updateData, bool& show);
};
