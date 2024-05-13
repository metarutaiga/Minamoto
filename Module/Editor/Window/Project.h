//==============================================================================
// Minamoto : Project Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct Project
{
    static std::string Root;
    static std::string SubFolder;
public:
    static void Initialize();
    static void Shutdown(bool suspend = false);
    static bool Update(const UpdateData& updateData, bool& show);
};
