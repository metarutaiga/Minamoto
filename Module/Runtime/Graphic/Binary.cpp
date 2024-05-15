//==============================================================================
// Minamoto : Binary Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>
#include <xxGraphicPlus/xxFile.h>
#include "Binary.h"

//==============================================================================
//  Binary
//==============================================================================
Binary::Binary()
{
}
//------------------------------------------------------------------------------
Binary::~Binary()
{
}
//------------------------------------------------------------------------------
xxNodePtr Binary::Load(char const* name)
{
    xxNodePtr node;

    xxFile* file = xxFile::Load(name);
    if (file)
    {
        Binary binary;

        binary.m_file = file;
        binary.m_reference.resize(1);
        const_cast<std::string&>(binary.Path) = xxFile::GetPath(name);
        binary.m_stringStream.resize(1);

        char signature[12];
        if (file->Read(signature, 12) &&
            file->Read(const_cast<int*>(&binary.Version), 4) &&
            strcmp(signature, xxBINARY_SIGNATURE) == 0)
        {
            if (binary.Version == binary.xxBinary::Version)
            {
                delete file;
                auto output = xxBinary::Load(name);
                return (xxNodePtr&)output;
            }

            if (binary.ReadStream())
            {
                auto output = xxNode::BinaryCreate();
                node = (xxNodePtr&)output;
            }
            if (node)
            {
                node->BinaryRead(binary);
            }
            if (node == nullptr || binary.Safe == false)
            {
                node = nullptr;
            }
        }

        delete file;
        file = nullptr;
    }

    return node;
}
//------------------------------------------------------------------------------
bool Binary::Save(char const* name, xxNodePtr const& node)
{
    bool succeed = false;

    xxFile* file = xxFile::Save(name);
    if (file)
    {
        Binary binary;

        binary.m_file = file;
        binary.m_reference.resize(1);
        const_cast<std::string&>(binary.Path) = xxFile::GetPath(name);
        binary.m_stringStream.resize(1);

        char signature[12] = xxBINARY_SIGNATURE;
        if (file->Write(signature, 12) &&
            file->Write(&binary.Version, 4))
        {
            if (node)
            {
                node->BinaryWrite(binary);
            }
            if (binary.Safe)
            {
                succeed = binary.WriteStream();
            }
        }

        delete file;
        file = nullptr;
    }

    return succeed;
}
//------------------------------------------------------------------------------
bool Binary::ReadStream()
{
    size_t position = m_file->Position();
    size_t size = m_file->Size();
    if (position >= size)
        return false;
    m_binaryStream.resize(size - position);
    if (m_file->Read(m_binaryStream.data(), m_binaryStream.size()) == false)
        return false;
    m_binaryStream.push_back(0);

    for (;;)
    {
        std::string string = (char*)m_binaryStream.data() + m_binaryStreamPosition;
        if (string.empty())
            break;
        m_stringStream.push_back(string);
        m_binaryStreamPosition += string.length() + 1;
    }
    m_binaryStreamPosition++;

    return true;
}
//------------------------------------------------------------------------------
bool Binary::WriteStream()
{
    std::vector<uint8_t> binaryStream;
    m_binaryStream.swap(binaryStream);

    for (size_t i = 1; i < m_stringStream.size(); ++i)
    {
        const std::string& string = m_stringStream[i];
        if (string.empty())
            break;
        if (WriteArray(string.data(), string.length() + 1) == false)
            return false;
    }
    m_binaryStream.push_back(0);

    if (m_file->Write(m_binaryStream.data(), m_binaryStream.size()) == false)
        return false;
    if (m_file->Write(binaryStream.data(), binaryStream.size()) == false)
        return false;
    return true;
}
//------------------------------------------------------------------------------
bool Binary::Read(void* data, size_t size)
{
    m_called++;
    if (m_binaryStream.size() < m_binaryStreamPosition + size)
    {
        m_failed = m_called;
        const_cast<bool&>(Safe) = false;
        return false;
    }
    memcpy(data, m_binaryStream.data() + m_binaryStreamPosition, size);
    m_binaryStreamPosition += size;
    return true;
}
//------------------------------------------------------------------------------
bool Binary::Write(void const* data, size_t size)
{
    m_called++;
    m_binaryStream.insert(m_binaryStream.end(), (uint8_t*)data, (uint8_t*)data + size);
    return true;
}
//------------------------------------------------------------------------------
bool Binary::ReadString(std::string& string)
{
    size_t index = 0;
    if (ReadSize(index) == false)
        return false;
    if (m_stringStream.size() <= index)
        return false;
    string = m_stringStream[index];
    return true;
}
//------------------------------------------------------------------------------
bool Binary::WriteString(std::string const& string)
{
    for (size_t i = 0; i < m_stringStream.size(); ++i)
    {
        if (m_stringStream[i] == string)
        {
            return WriteSize(i);
        }
    }
    size_t index = m_stringStream.size();
    if (WriteSize(index) == false)
        return false;
    m_stringStream.push_back(string);
    return true;
}
//==============================================================================
