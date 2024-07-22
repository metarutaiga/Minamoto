//==============================================================================
// Minamoto : ShaderAssemblerD3D8 Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct ShaderAssemblerD3D8
{
    static std::vector<uint32_t> Assemble(std::string const& shader, std::string& message);
    static std::string Disassemble(std::vector<uint32_t> const& code, std::string& message);
};
