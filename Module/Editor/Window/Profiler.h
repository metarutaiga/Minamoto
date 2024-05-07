//==============================================================================
// Minamoto : Profiler Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

class Profiler
{
public:
    static void Initialize();
    static void Shutdown();
    static bool Update(const UpdateData& updateData, bool& show);
    static void Begin(unsigned int hashName);
    static void End(unsigned int hashName);
    static void Count(unsigned int hashName, size_t count);
};
