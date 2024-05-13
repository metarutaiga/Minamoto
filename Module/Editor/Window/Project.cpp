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
#include <xxGraphic/utility/xxTexture.h>
#include <Runtime/Graphic/Texture.h>
#include <imgui/imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include "Hierarchy.h"
#include "Component/Folder.h"
#include "Project.h"

std::string Project::Root;
std::string Project::SubFolder;
//------------------------------------------------------------------------------
static Folder Left;
//------------------------------------------------------------------------------
struct FileAttribute
{
    std::string name;
    xxTexturePtr texture;
};
static xxTexturePtr DummyTexture;
static std::vector<FileAttribute> Files;
//------------------------------------------------------------------------------
void Project::Initialize()
{
    if (Root.empty())
    {
        Root = xxGetDocumentPath();
        Root += "/Project/";
    }
    DummyTexture = xxTexture::Create("RGBA8888"_FOURCC, 1, 1, 1, 1, 1);
}
//------------------------------------------------------------------------------
void Project::Shutdown(bool suspend)
{
    DummyTexture = nullptr;
    for (auto& attribute : Files)
    {
        if (attribute.texture)
        {
            attribute.texture->Initialize(0, 0, 0, 0, 0, 0);
            attribute.texture->Invalidate();
        }
    }
    if (suspend)
        return;
    Root.clear();
    SubFolder.clear();
    Files.clear();
}
//------------------------------------------------------------------------------
static void ShowFolders(std::string const& root, std::string& select)
{
    if (ImGui::IsWindowAppearing() && root.empty() == false)
    {
        Left.Finder(root);
    }
    Left.Window([&](std::string const& root, std::string const& subfolder)
    {
        select = subfolder;

        Files.clear();
        uint64_t handle = 0;
        std::string folder = root + subfolder;
        while (char* filename = xxOpenDirectory(&handle, folder.c_str(), nullptr))
        {
            if (filename[0] != '.' && strstr(filename, "/") == nullptr)
            {
                xxTexturePtr texture;
                if (strcasestr(filename, ".dds") || strcasestr(filename, ".png"))
                {
                    texture = xxTexture::Create(0, 0, 0, 0, 0, 0);
                    texture->Name = filename;
                }
                Files.push_back({filename, texture});
            }
            xxFree(filename);
        }
        xxCloseDirectory(&handle);
        std::sort(Files.begin(), Files.end(), [](auto& a, auto& b) { return a.name < b.name; });
    });
}
//------------------------------------------------------------------------------
static void ShowFiles(const UpdateData& updateData, std::string const& root, std::string const& subfolder, std::vector<FileAttribute> const& files)
{
    bool textureUpdate = false;
    float width = 96.0f;
    int columns = int(ImGui::GetColumnWidth() / (width + ImGui::GetStyle().FramePadding.x * 2.0f));
    if (columns < 1)
        columns = 1;

    ImGui::BeginChild("###", ImVec2(0, 0), 0, Files.empty() ? 0 : ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Columns(columns, nullptr, false);
    for (auto& attribute : Files)
    {
        uint64_t texture = 0;
        if (attribute.texture)
        {
            if ((*attribute.texture)() == nullptr && textureUpdate == false)
            {
                Texture::Reader(attribute.texture, root + subfolder);
                attribute.texture->Update(updateData.device);
                textureUpdate = (*attribute.texture)() != nullptr;
            }
            texture = attribute.texture->Texture;
        }
        if (texture == 0)
        {
            if (DummyTexture->Texture == 0)
                DummyTexture->Update(updateData.device);
            texture = DummyTexture->Texture;
        }
        ImGui::BeginGroup();
        ImGui::ImageButton(attribute.name.c_str(), texture, ImVec2(width, width));
        if (ImGui::IsItemHovered())
        {
            if (attribute.texture && attribute.texture->Format)
            {
                if (ImGui::BeginTooltip())
                {
                    if (attribute.texture->Width > 1)
                        ImGui::Text("Width : %d", attribute.texture->Width);
                    if (attribute.texture->Height > 1)
                        ImGui::Text("Height : %d", attribute.texture->Height);
                    if (attribute.texture->Depth > 1)
                        ImGui::Text("Depth : %d", attribute.texture->Depth);
                    if (attribute.texture->Mipmap > 1)
                        ImGui::Text("Mipmap : %d", attribute.texture->Mipmap);
                    if (attribute.texture->Array > 1)
                        ImGui::Text("Array : %d", attribute.texture->Array);
                    ImGui::Text("Format : %.8s", (char*)&attribute.texture->Format);
                    ImGui::EndTooltip();
                }
            }
        }
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
        {
            std::string file = root + subfolder + attribute.name;
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
        ShowFolders(Root, SubFolder);

        ImGui::NextColumn();

        ImGui::TextUnformatted(SubFolder.c_str());
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
        ShowFiles(updateData, Root, SubFolder, Files);

        ImGui::Columns(1);
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
