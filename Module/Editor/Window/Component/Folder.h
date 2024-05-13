//==============================================================================
// Minamoto : Folder Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct Folder
{
    struct Node : public std::vector<std::pair<std::string, Node>> {};

    Node Folders;
    std::string Root;
    void const* Selected = nullptr;

private:
    void Finder(std::string const& folder, Node& node);
    void Window(Node& node, std::function<void(std::string const& root, std::string const& subfolder)> select);

public:
    void Finder(std::string const& root);
    void Window(std::function<void(std::string const& root, std::string const& subfolder)> select);
};
