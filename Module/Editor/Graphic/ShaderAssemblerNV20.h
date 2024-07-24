//==============================================================================
// Minamoto : ShaderAssemblerNV20 Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct ShaderAssemblerNV20
{
    static int DebugPrintf(bool breakline, char const* format, ...);
    static std::vector<uint32_t> CompileCheops(std::vector<uint32_t> const& shader, std::string& message);
    static std::vector<uint32_t> CompileKelvin(std::vector<uint32_t> const& shader, std::string& message);
    static std::string DisassembleCheops(std::vector<uint32_t> const& code);
    static std::string DisassembleKelvin(std::vector<uint32_t> const& code);
};
