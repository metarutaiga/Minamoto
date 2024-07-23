//==============================================================================
// Minamoto : ShaderAssemblerNV10 Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct ShaderAssemblerNV10
{
    static std::vector<uint32_t> CompileCheops(std::vector<uint32_t> const& binary, std::string& message);
    static std::vector<uint32_t> CompileCelsius(std::vector<uint32_t> const& binary, std::string& message);
    static std::string DisassembleCheops(std::vector<uint32_t> const& code);
    static std::string DisassembleCelsius(std::vector<uint32_t> const& code);
};
