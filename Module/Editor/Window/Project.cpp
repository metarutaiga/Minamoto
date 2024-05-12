//==============================================================================
// Minamoto : Project Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <functional>
#include <string>
#include <vector>
#include <xxGraphic/utility/xxImage.h>
#include <Runtime/Graphic/Texture.h>
#include <imgui/imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include "Hierarchy.h"
#include "Project.h"

std::string Project::Root;
std::string Project::Current;
//------------------------------------------------------------------------------
struct Node : public std::vector<std::pair<std::string, Node>> {};
struct FileAttribute
{
    std::string name;
    xxImagePtr image;
};
static xxImagePtr DummyTexture;
static Node Folders;
static std::vector<FileAttribute> Files;
//------------------------------------------------------------------------------
void Project::Initialize()
{
    if (Root.empty())
    {
        Root = xxGetDocumentPath();
        Root += "/Project/";
    }
    DummyTexture = xxImage::Create("RGBA8888"_FOURCC, 1, 1, 1, 1, 1);
}
//------------------------------------------------------------------------------
void Project::Shutdown(bool suspend)
{
    DummyTexture = nullptr;
    for (auto& attribute : Files)
    {
        if (attribute.image)
        {
            attribute.image->Initialize(0, 0, 0, 0, 0, 0);
            attribute.image->Invalidate();
        }
    }
    if (suspend)
        return;
    Root.clear();
    Current.clear();
    Folders.clear();
    Files.clear();
}
//------------------------------------------------------------------------------
static void Finder(std::string const& folder, Node& node)
{
    uint64_t handle = 0;
    while (char* filename = xxOpenDirectory(&handle, folder.c_str(), nullptr))
    {
        if (filename[0] != '.' && strstr(filename, "/"))
        {
            node.push_back({std::string(ICON_FA_FOLDER) + filename, Node()});
            node.back().first.pop_back();
            Finder(folder + filename, node.back().second);
        }
        xxFree(filename);
    }
    xxCloseDirectory(&handle);
}
//------------------------------------------------------------------------------
static void ShowFolders(std::string const& root, std::string& current, Node& node)
{
    static Node const* selected;
    static std::vector<std::string> stacked;
    for (auto& [name, directory] : node)
    {
        bool open = name.compare(0, sizeof(ICON_FA_FOLDER) - 1, ICON_FA_FOLDER) != 0;
        ImGui::Selectable(name.c_str(), selected == &directory);
        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsItemClicked() && selected != &directory)
            {
                selected = &directory;
                current.clear();
                for (auto const& name : stacked)
                {
                    current += name.c_str() + sizeof(ICON_FA_FOLDER) - 1;
                    current += '/';
                }
                current += name.c_str() + sizeof(ICON_FA_FOLDER) - 1;
                Files.clear();
                uint64_t handle = 0;
                std::string folder = root + current;
                while (char* filename = xxOpenDirectory(&handle, folder.c_str(), nullptr))
                {
                    if (filename[0] != '.' && strstr(filename, "/") == nullptr)
                    {
                        xxImagePtr image;
                        if (strcasestr(filename, ".dds") || strcasestr(filename, ".png"))
                        {
                            image = xxImage::Create(0, 0, 0, 0, 0, 0);
                            image->Name = filename;
                        }
                        Files.push_back({filename, image});
                    }
                    xxFree(filename);
                }
                xxCloseDirectory(&handle);
                std::sort(Files.begin(), Files.end(), [](auto& a, auto& b) { return a.name < b.name; });
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
                }
            }
        }
        if (open)
        {
            ImGui::Indent();
            stacked.push_back(name);
            ShowFolders(root, current, directory);
            stacked.pop_back();
            ImGui::Unindent();
        }
    }
}
//------------------------------------------------------------------------------
static void ShowFiles(const UpdateData& updateData, std::string const& root, std::string const& current, std::vector<FileAttribute> const& files)
{
    bool imageUpdate = false;
    float width = 96.0f;
    int columns = int(ImGui::GetColumnWidth() / (width + ImGui::GetStyle().FramePadding.x * 2.0f));
    if (columns < 1)
        columns = 1;

    ImGui::BeginChild("###", ImVec2(0, 0), 0, Files.empty() ? 0 : ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Columns(columns, nullptr, false);
    for (auto& attribute : Files)
    {
        uint64_t texture = 0;
        if (attribute.image)
        {
            if ((*attribute.image)() == nullptr && imageUpdate == false)
            {
                Texture::Reader(attribute.image, root + current + '/');
                attribute.image->Update(updateData.device);
                imageUpdate = (*attribute.image)() != nullptr;
            }
            texture = attribute.image->Texture;
        }
        if (texture == 0)
        {
            if (DummyTexture->Texture == 0)
                DummyTexture->Update(updateData.device);
            texture = DummyTexture->Texture;
        }
        ImGui::BeginGroup();
        ImGui::ImageButton(attribute.name.c_str(), texture, ImVec2(width, width));
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
        {
            std::string file = root + current + '/' + attribute.name;
            ImGui::TextUnformatted(attribute.name.c_str());
            ImGui::SetDragDropPayload("DRAGFILE", file.data(), file.size());
            ImGui::EndDragDropSource();
        }
        ImGui::TextWrapped("%s", attribute.name.c_str());
        ImGui::EndGroup();
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::EndChild();
}
//------------------------------------------------------------------------------
bool Project::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    if (ImGui::Begin(ICON_FA_BOOKMARK "Project", &show))
    {
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 256.0f);

        if (ImGui::IsWindowAppearing() && Root.empty() == false)
        {
            Folders.clear();
            Folders.push_back({"", Node()});
            Finder(Root, Folders.back().second);
        }

        if (Folders.empty() == false)
        {
            ShowFolders(Root, Current, Folders.back().second);
        }

        ImGui::NextColumn();

        ImGui::TextUnformatted(Current.c_str());
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
        ShowFiles(updateData, Root, Current, Files);

        ImGui::Columns(1);
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
