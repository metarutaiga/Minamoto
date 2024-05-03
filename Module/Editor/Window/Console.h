//==============================================================================
// Minamoto : Console Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct Console
{
    void Update(const UpdateData& updateData);
    void AddHistory(const char* line);

    std::string input;
    std::vector<std::string> history;
    size_t inputPos = 0;
    size_t historyPos = 0;
};
