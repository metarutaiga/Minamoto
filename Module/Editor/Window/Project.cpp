//==============================================================================
// Minamoto : Project Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <algorithm>
#include <functional>
#include <string>
#include <vector>
#include <xxGraphicPlus/xxTexture.h>
#include <Runtime/Graphic/Texture.h>
#include "Component/Folder.h"
#include "Utility/TextureTools.h"
#include "Hierarchy.h"
#include "Project.h"
#include "Setup.h"

std::string Project::Root;
std::string Project::SubFolder;
//------------------------------------------------------------------------------
static Folder Left;
//------------------------------------------------------------------------------
struct FileAttribute
{
    std::string name;
    xxTexturePtr texture;
    bool textureLoaded = false;
};
static xxTexturePtr DummyTexture;
static std::vector<FileAttribute> Files;
//------------------------------------------------------------------------------
static bool SetupWindow;
static Folder SetupFolder;
static std::string SetupCurrent;
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
            attribute.textureLoaded = false;
        }
    }
    if (suspend)
        return;
    Root.clear();
    SubFolder.clear();
    Files.clear();
}
//------------------------------------------------------------------------------
static bool ShowSetup(bool& show)
{
    if (show == false)
        return false;

    bool select = false;
    if (ImGui::Begin("Change project path", &show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        SetupFolder.Window([&](std::string const& root, std::string const& subfolder)
        {
            SetupCurrent = root + subfolder;
        });
        if (SetupCurrent.empty() == false && ImGui::Button("OK"))
        {
            Project::Root = std::move(SetupCurrent);
            show = false;
            select = true;

            Setup::Save();
        }
    }
    ImGui::End();

    return select;
}
//------------------------------------------------------------------------------
static void ShowFolders(std::string const& root, std::string& selected)
{
    auto select = [&](std::string const& root, std::string const& subfolder)
    {
        selected = subfolder;

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
                    texture = xxTexture::Create();
                    texture->Name = filename;
                    texture->Path = folder;
                }
                Files.push_back({ filename, texture });
            }
            xxFree(filename);
        }
        xxCloseDirectory(&handle);
        std::sort(Files.begin(), Files.end(), [](auto& a, auto& b) { return a.name < b.name; });
    };

    if (ImGui::IsWindowAppearing() && root.empty() == false)
    {
        Left.Finder(root);
        if (Files.empty())
        {
            Left.Selected = nullptr;
            select(root, "");
        }
    }
    Left.Window(select);

    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x && avail.y)
    {
        if (ImGui::InvisibleButton("", avail))
        {
            Left.Selected = nullptr;
            select(root, "");
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup("PopupProjectSetup");
        }
    }

    if (ImGui::BeginPopup("PopupProjectSetup"))
    {
        if (ImGui::Button("Change project path"))
        {
            std::string root = std::string(xxGetDocumentPath()) + '/';
            SetupFolder.Finder(root);
            SetupWindow = true;
        }
        ImGui::EndPopup();
    }

    if (ShowSetup(SetupWindow))
    {
        SetupFolder.Selected = nullptr;
        Left.Selected = nullptr;
        Left.Finder(root);
        select(root, "");
    }
}
//------------------------------------------------------------------------------
static void ShowFiles(const UpdateData& updateData, std::string const& root, std::string const& subfolder, std::vector<FileAttribute> const& files)
{
    bool textureUpdate = false;
    float width = 96.0f;
    int columns = int(ImGui::GetColumnWidth() / (width + ImGui::GetStyle().FramePadding.x * 4.0f));
    if (columns < 1)
        columns = 1;

    ImGui::BeginChild("###", ImVec2(0, 0), 0, Files.empty() ? 0 : ImGuiWindowFlags_AlwaysVerticalScrollbar);
    ImGui::Columns(columns, nullptr, false);
    for (auto& attribute : Files)
    {
        uint64_t texture = 0;
        if (attribute.texture)
        {
            if (attribute.texture->Texture == 0 && attribute.textureLoaded == false && textureUpdate == false)
            {
                attribute.texture->Update(updateData.device);
                attribute.textureLoaded = true;
                textureUpdate = true;
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
        static void* selected = nullptr;
        if (selected == &attribute && ImGui::BeginPopup("texture"))
        {
            uint64_t format = 0;
            if (ImGui::Button("Compress BC1"))  format = "BC1"_FOURCC;
            if (ImGui::Button("Compress BC2"))  format = "BC2"_FOURCC;
            if (ImGui::Button("Compress BC3"))  format = "BC3"_FOURCC;
            if (ImGui::Button("Compress BC4U")) format = "BC4U"_FOURCC;
            if (ImGui::Button("Compress BC5U")) format = "BC5U"_FOURCC;
            if (format)
            {
                TextureTools::CompressTexture(attribute.texture, format, root, subfolder);
                selected = nullptr;
            }
            ImGui::EndPopup();
        }
        if (ImGui::IsItemHovered())
        {
            if (attribute.texture && attribute.texture->Format)
            {
                if (attribute.texture->Format == "RGBA8888"_FOURCC && ImGui::IsItemClicked(ImGuiMouseButton_Right))
                {
                    ImGui::OpenPopup("texture");
                    selected = &attribute;
                }
                if (ImGui::BeginTooltip())
                {
                    ImGui::Text("Format : %.8s", (char*)&attribute.texture->Format);
                    if (attribute.texture->Width > 1)   ImGui::Text("Width : %d", attribute.texture->Width);
                    if (attribute.texture->Height > 1)  ImGui::Text("Height : %d", attribute.texture->Height);
                    if (attribute.texture->Depth > 1)   ImGui::Text("Depth : %d", attribute.texture->Depth);
                    if (attribute.texture->Mipmap > 1)  ImGui::Text("Mipmap : %d", attribute.texture->Mipmap);
                    if (attribute.texture->Array > 1)   ImGui::Text("Array : %d", attribute.texture->Array);
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
