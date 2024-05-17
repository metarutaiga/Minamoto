//==============================================================================
// Minamoto : CSV Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxFile.h>
#include "CSV.h"

//==============================================================================
bool CSV::Load(char const* name, std::function<void(std::vector<std::string_view> const&)> deserialize, std::string_view separate)
{
    xxFile* file = xxFile::Load(name);
    if (file == nullptr)
        return false;

    std::string buffer;
    std::vector<std::string_view> rows;
    for (;;)
    {
        // Read
        size_t position = buffer.size();
        buffer.resize(position + 256);
        size_t length = file->Read(buffer.data() + position, buffer.size() - position);
        buffer.resize(position + length);
        if (buffer.empty())
            break;

        for (;;)
        {
            size_t breakline = buffer.find_first_of("\r\n");
            if (breakline == std::string::npos)
                break;

            // Find comma
            std::string_view line = std::string_view(buffer.c_str(), breakline);
            while (line.empty() == false)
            {
                size_t comma = std::min(line.size(), line.find_first_of(separate));
                rows.push_back(line.substr(0, comma));
                line.remove_prefix(std::min(line.size(), comma + 1));
            }
            deserialize(rows);
            rows.clear();

            // Skip breakline
            while (buffer.size() > breakline)
            {
                char c = buffer[breakline];
                if (c != '\r' && c != '\n')
                    break;
                breakline++;
            }
            buffer.erase(buffer.begin(), buffer.begin() + breakline);
        }
    }

    delete file;
    return true;
}
//------------------------------------------------------------------------------
bool CSV::Save(char const* name, std::function<void(std::vector<std::string_view>&)> serialize, std::string_view separate)
{
    xxFile* file = xxFile::Save(name);
    if (file == nullptr)
        return false;

    std::vector<std::string_view> rows;
    for (;;)
    {
        serialize(rows);
        if (rows.empty())
            break;

        for (size_t i = 0; i < rows.size(); ++i)
        {
            if (i != 0)
            {
                file->Write(separate.data(), separate.size());
            }
            std::string_view const& string = rows[i];
            file->Write(string.data(), string.size());
        }
        rows.clear();

        file->Write("\n", 1);
    }

    delete file;
    return true;
}
//==============================================================================
