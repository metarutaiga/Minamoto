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
    Root = xxGetDocumentPath();
    Root += "/Project/";
    DummyTexture = xxImage::Create("RGBA8888"_FOURCC, 1, 1, 1, 1, 1);
}
//------------------------------------------------------------------------------
void Project::Shutdown()
{
    Root.clear();
    Current.clear();
    DummyTexture = nullptr;
    Folders.clear();
    Files.clear();
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

        if (ImGui::IsWindowAppearing())
        {
            std::function<void(std::string const&, Node&)> finder = [&](std::string const& path, Node& node)
            {
                uint64_t handle = 0;
                std::string folder = Root + path;
                while (char* filename = xxOpenDirectory(&handle, folder.c_str(), nullptr))
                {
                    if (filename[0] != '.' && strstr(filename, "/"))
                    {
                        node.push_back({std::string(ICON_FA_FOLDER) + filename, Node()});
                        node.back().first.pop_back();
                        finder(path + filename, node.back().second);
                    }
                    xxFree(filename);
                }
                xxCloseDirectory(&handle);
            };
            Folders.clear();
            if (Root.empty() == false)
            {
                Folders.push_back({"", Node()});
                finder("", Folders.back().second);
            }
        }

        if (Folders.empty() == false)
        {
            static Node const* selected;
            static std::vector<std::string> stacked;
            std::function<void(Node&)> recursive = [&](Node& node)
            {
                for (auto& [name, directory] : node)
                {
                    bool open = name.compare(0, sizeof(ICON_FA_FOLDER) - 1, ICON_FA_FOLDER) != 0;
                    ImGui::Selectable(name.c_str(), selected == &directory);
                    if (ImGui::IsItemHovered())
                    {
                        if (ImGui::IsItemClicked() && selected != &directory)
                        {
                            selected = &directory;
                            Current.clear();
                            for (auto const& name : stacked)
                            {
                                Current += name.c_str() + sizeof(ICON_FA_FOLDER) - 1;
                                Current += '/';
                            }
                            Current += name.c_str() + sizeof(ICON_FA_FOLDER) - 1;
                            Files.clear();
                            uint64_t handle = 0;
                            std::string folder = Root + Current;
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
                        recursive(directory);
                        stacked.pop_back();
                        ImGui::Unindent();
                    }
                }
            };
            recursive(Folders.back().second);
        }

        ImGui::NextColumn();

        if (DummyTexture->Texture == 0)
            DummyTexture->Update(updateData.device);

        ImGui::TextUnformatted(Current.c_str());
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);

        ImGui::BeginChild("###", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);
        bool imageUpdate = false;
        float width = 96.0f;
        int columns = int(ImGui::GetColumnWidth() / (width + ImGui::GetStyle().FramePadding.x * 2.0f));
        if (columns < 1)
            columns = 1;
        ImGui::Columns(columns, nullptr, false);
        for (auto& attribute : Files)
        {
            uint64_t texture = 0;
            if (attribute.image)
            {
                if ((*attribute.image)() == nullptr && imageUpdate == false)
                {
                    Texture::Reader(attribute.image, Root + Current + '/');
                    attribute.image->Update(updateData.device);
                    imageUpdate = (*attribute.image)() != nullptr;
                }
                texture = attribute.image->Texture;
            }
            if (texture == 0)
            {
                texture = DummyTexture->Texture;
            }
            ImGui::BeginGroup();
            ImGui::ImageButton(attribute.name.c_str(), texture, ImVec2(width, width));
            ImGui::TextWrapped("%s", attribute.name.c_str());
            ImGui::EndGroup();
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::EndChild();

        ImGui::Columns(1);
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
