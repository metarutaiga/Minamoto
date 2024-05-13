//==============================================================================
// Minamoto : Folder Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <functional>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include "Folder.h"

//------------------------------------------------------------------------------
void Folder::Finder(std::string const& folder, Node& node)
{
    uint64_t handle = 0;
    while (char* filename = xxOpenDirectory(&handle, folder.c_str(), nullptr))
    {
        if (filename[0] != '.' && strstr(filename, "/"))
        {
            node.push_back({std::string(ICON_FA_FOLDER) + filename, Node()});
            node.back().first.pop_back();
        }
        xxFree(filename);
    }
    xxCloseDirectory(&handle);
}
//------------------------------------------------------------------------------
void Folder::Window(Node& node, std::function<void(std::string const& root, std::string const& subfolder)> select)
{
    static std::vector<std::string> stacked;
    auto SubFolder = [this](std::string const& name)
    {
        std::string path;
        for (auto const& name : stacked)
        {
            path += name.c_str() + sizeof(ICON_FA_FOLDER) - 1;
            path += '/';
        }
        path += name.c_str() + sizeof(ICON_FA_FOLDER) - 1;
        path += '/';
        return path;
    };

    for (auto& [name, directory] : node)
    {
        bool open = name.compare(0, sizeof(ICON_FA_FOLDER) - 1, ICON_FA_FOLDER) != 0;
        ImGui::Selectable(name.c_str(), Selected == &directory);
        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsItemClicked() && Selected != &directory)
            {
                Selected = &directory;
                select(Root, SubFolder(name));
            }
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (open)
                {
                    name.replace(0, sizeof(ICON_FA_FOLDER) - 1, ICON_FA_FOLDER);
                }
                else
                {
                    name.replace(0, sizeof(ICON_FA_FOLDER_OPEN) - 1, ICON_FA_FOLDER_OPEN);

                    directory.clear();
                    Finder(Root + SubFolder(name), directory);
                }
            }
        }
        if (open)
        {
            ImGui::Indent();
            stacked.push_back(name);
            Window(directory, select);
            stacked.pop_back();
            ImGui::Unindent();
        }
    }
}
//------------------------------------------------------------------------------
void Folder::Finder(std::string const& root)
{
    Folders.clear();
    Folders.push_back({"", Node()});
    Root = root;
    Finder(root, Folders.back().second);
}
//------------------------------------------------------------------------------
void Folder::Window(std::function<void(std::string const& root, std::string const& subfolder)> select)
{
    if (Folders.empty())
        return;
    Window(Folders.back().second, select);
}
//------------------------------------------------------------------------------
