//==============================================================================
// Minamoto : Binary Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Runtime.h"
#include <xxGraphicPlus/xxBinary.h>

class RuntimeAPI Binary : public xxBinary
{
public:
    static xxNodePtr            Load(char const* name);
    static bool                 Save(char const* name, xxNodePtr const& node);

protected:
    Binary();
    virtual ~Binary();

    bool                        Read(void* data, size_t size) override;
    bool                        Write(void const* data, size_t size) override;

    bool                        ReadStream();
    bool                        WriteStream();

    size_t                      m_binaryStreamPosition = 0;
    std::vector<uint8_t>        m_binaryStream;
    std::vector<std::string>    m_stringStream;

public:
    bool                        ReadString(std::string& string) override;
    bool                        WriteString(std::string const& string) override;

    int const                   Version = 0x20240503;
};
