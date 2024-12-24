//==============================================================================
// Minamoto : Hierarchy Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <xxGraphicPlus/xxCamera.h>
#include <xxGraphicPlus/xxFile.h>
#include <xxGraphicPlus/xxNode.h>
#include <Graphic/Binary.h>
#include <MiniGUI/Window.h>
#include <Runtime/Tools/NodeTools.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include "Import/ImportFBX.h"
#include "Import/ImportPLY.h"
#include "Import/ImportWavefront.h"
#include "Utility/Tools.h"
#include "Hierarchy.h"
#include "Log.h"
#include "Scene.h"
#include "Inspector.h"

#if defined(__APPLE__)
#define HAVE_FILEDIALOG 1
#elif defined(_WIN32) && !defined(__clang__)
#define HAVE_FILEDIALOG 1
#else
#define HAVE_FILEDIALOG 0
#endif

#define TAG "Hierarchy"

float Hierarchy::windowPosY = 0.0f;
float Hierarchy::windowWidth = 256.0f;
xxNodePtr Hierarchy::selectedLeft;
xxNodePtr Hierarchy::selectedRight;
xxNodePtr Hierarchy::importNode;
xxNodePtr Hierarchy::exportNode;
char Hierarchy::importName[1024];
char Hierarchy::exportName[1024];
ImGuiFileDialog* Hierarchy::importFileDialog;
ImGuiFileDialog* Hierarchy::exportFileDialog;
//==============================================================================
void Hierarchy::Initialize()
{
    selectedLeft = nullptr;
    selectedRight = nullptr;
    importNode = nullptr;
    exportNode = nullptr;
#if HAVE_FILEDIALOG
    importFileDialog = new ImGuiFileDialog;
    exportFileDialog = new ImGuiFileDialog;
#endif
}
//------------------------------------------------------------------------------
void Hierarchy::Shutdown()
{
#if HAVE_FILEDIALOG
    delete importFileDialog;
    delete exportFileDialog;
#endif
    selectedLeft = nullptr;
    selectedRight = nullptr;
    importNode = nullptr;
    exportNode = nullptr;
    importFileDialog = nullptr;
    exportFileDialog = nullptr;
}
//------------------------------------------------------------------------------
static xxNodePtr ImportFile(char const* name)
{
    float begin = xxGetCurrentTime();
    xxNodePtr node;
    if (strcasestr(name, ".fbx"))
        node = ImportFBX::Create(name);
    if (strcasestr(name, ".obj"))
        node = ImportWavefront::Create(name);
    if (strcasestr(name, ".ply"))
        node = ImportPLY::Create(name);
    if (strcasestr(name, ".xxb"))
        node = Binary::Load(name);
    if (node)
    {
        float time = xxGetCurrentTime() - begin;
        xxLog("Hierarchy", "Import : %s (%0.fus)", xxFile::GetName(name).c_str(), time * 1000000);
    }
    return node;
}
//------------------------------------------------------------------------------
void Hierarchy::Select(xxNodePtr const& node)
{
    selectedLeft = node;
}
//------------------------------------------------------------------------------
void Hierarchy::Import(const UpdateData& updateData)
{
    if (importNode == nullptr)
        return;

    bool show = true;
    if (ImGui::Begin("Import", &show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SetNextItemWidth(384 * updateData.scale);
        ImGui::InputText("Path", importName, 1024);
        ImGui::SameLine();
        if (ImGui::Button("..."))
        {
#if HAVE_FILEDIALOG
            if (importName[0] == 0 && exportName[0])
                strcpy(importName, exportName);
#if defined(_WIN32)
            IGFD::FileDialogConfig config = { importName[0] ? importName : std::string(xxGetDocumentPath()) + '\\' };
            if (config.path.size() && config.path.back() != '\\')
                config.path.resize(config.path.rfind('\\') + 1);
#else
            IGFD::FileDialogConfig config = { importName[0] ? importName : std::string(xxGetDocumentPath()) + '/' };
            if (config.path.size() && config.path.back() != '/')
                config.path.resize(config.path.rfind('/') + 1);
#endif
            char const* filters =
                "Supported Files(*.fbx,*.obj,*.xxb){.fbx,.obj,.xxb},"
                "Double Cross Binary(*.xxb){.xxb},"
                "Kaydara Flimbox(*.fbx){.fbx},"
                "Wavefront Object(*.obj){.obj},"
                "All Files(*.*){.*},";
            importFileDialog->OpenDialog("Import", "Choose File", filters, config);
#endif
        }
        ImGui::Checkbox("Axis Up Y to Z", &Import::EnableAxisUpYToZ);
        ImGui::Checkbox("Merge Node", &Import::EnableMergeNode);
        ImGui::Checkbox("Merge Texture", &Import::EnableMergeTexture);
        ImGui::Checkbox("Optimize Mesh", &Import::EnableOptimizeMesh);
        ImGui::Checkbox("Texture Flip V", &Import::EnableTextureFlipV);
        if (ImGui::Button("Import"))
        {
            xxNodePtr node = ImportFile(importName);
            if (node)
            {
                if (Import::EnableMergeNode)
                {
                    Import::MergeNode(importNode, node, importNode);
                }
                else
                {
                    importNode->AttachChild(node);
                }
                if (Import::EnableMergeTexture)
                {
                    Import::MergeTexture(node);
                }

                xxNodePtr const& root = NodeTools::GetRoot(importNode);
                root->CreateLinearMatrix();

                importNode = nullptr;
                show = false;
            }
        }
    }
    ImGui::End();

#if HAVE_FILEDIALOG
    if (importFileDialog->Display("Import", 0, ImVec2(384 * updateData.scale, 256 * updateData.scale)))
    {
        if (importFileDialog->IsOk())
        {
            strcpy(importName, importFileDialog->GetFilePathName().c_str());
        }
        importFileDialog->Close();
    }
#endif

    if (show == false)
    {
        importNode = nullptr;
#if HAVE_FILEDIALOG
        importFileDialog->Close();
#endif
    }
}
//------------------------------------------------------------------------------
void Hierarchy::Export(const UpdateData& updateData)
{
    if (exportNode == nullptr)
        return;

    bool show = true;
    if (ImGui::Begin("Export", &show, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SetNextItemWidth(384 * updateData.scale);
        ImGui::InputText("Path", exportName, 1024);
        ImGui::SameLine();
        if (ImGui::Button("..."))
        {
#if HAVE_FILEDIALOG
            if (exportName[0] == 0 && importName[0])
                strcpy(exportName, importName);
#if defined(_WIN32)
            IGFD::FileDialogConfig config = { exportName[0] ? exportName : std::string(xxGetDocumentPath()) + '\\' };
            if (config.path.size() && config.path.back() != '\\')
                config.path.resize(config.path.rfind('\\') + 1);
#else
            IGFD::FileDialogConfig config = { exportName[0] ? exportName : std::string(xxGetDocumentPath()) + '/' };
            if (config.path.size() && config.path.back() != '/')
                config.path.resize(config.path.rfind('/') + 1);
#endif
            char const* filters =
                "Double Cross Binary(*.xxb){.xxb},"
                "All Files(*.*){.*},";
            exportFileDialog->OpenDialog("Export", "Choose File", filters, config);
#endif
        }
        if (ImGui::Button("Export"))
        {
            xxNode::Traversal(exportNode, [&](xxNodePtr const& node)
            {
                node->Flags &= ~NodeTools::TEST_CHECK_FLAG;
                return true;
            });
            float begin = xxGetCurrentTime();
            if (Binary::Save(exportName, exportNode))
            {
                float time = xxGetCurrentTime() - begin;
                xxLog("Hierarchy", "Export : %s (%0.fus)", xxFile::GetName(exportName).c_str(), time * 1000000);
                exportNode = nullptr;
                show = false;
            }
        }
    }
    ImGui::End();

#if HAVE_FILEDIALOG
    if (exportFileDialog->Display("Export", 0, ImVec2(384 * updateData.scale, 256 * updateData.scale)))
    {
        if (exportFileDialog->IsOk())
        {
            strcpy(exportName, exportFileDialog->GetFilePathName().c_str());
        }
        exportFileDialog->Close();
    }
#endif

    if (show == false)
    {
        exportNode = nullptr;
#if HAVE_FILEDIALOG
        exportFileDialog->Close();
#endif
    }
}
//------------------------------------------------------------------------------
bool Hierarchy::Update(const UpdateData& updateData, bool& show, xxNodePtr const& root)
{
    if (show == false)
        return false;

    bool update = false;
    xxNodePtr hovered;
    if (ImGui::Begin(ICON_FA_LIST "Hierarchy", &show))
    {
        windowPosY = ImGui::GetCursorPosY();
        windowWidth = ImGui::GetWindowWidth();

        auto dragFile = [](xxNodePtr const& node)
        {
            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAGFILE");
                if (payload)
                {
                    std::string name((char*)payload->Data, payload->DataSize);
                    xxNodePtr object = ImportFile(name.c_str());
                    if (object)
                    {
                        if (node->GetParent())
                        {
                            Import::MergeNode(node, object, node);
                        }
                        else
                        {
                            node->AttachChild(object);
                        }

                        xxNodePtr const& root = NodeTools::GetRoot(node);
                        root->CreateLinearMatrix();
                    }
                }
                ImGui::EndDragDropTarget();
            }
        };

        char name[256];
        bool clickedLeft = false;
        bool clickedRight = false;
        size_t TEST_OPEN_FLAG = xxNode::RESERVED0;
        std::function<void(xxNodePtr const&)> traversal = [&](xxNodePtr const& node)
        {
            if (node == nullptr)
                return;

            // TODO -
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                ImVec2 min = ImGui::GetCursorScreenPos();
                ImVec2 max;
                max.x = min.x + ImGui::GetContentRegionAvail().x;
                max.y = min.y + ImGui::GetFrameHeight();
                if (ImGui::IsMouseHoveringRect(min, max))
                {
                    ImGui::GetWindowDrawList()->AddRect(min, max, ImGui::GetColorU32(ImGuiCol_DragDropTarget), 0.0f, 0, 2.0f);
                }
            }

            bool opened = (node->Flags & TEST_OPEN_FLAG) != 0;
            char const* type;
            if (node->Camera)
                type = ICON_FA_CAMERA;
            else if (node->Mesh)
                type = ICON_FA_CUBE;
            else
                type = opened ? ICON_FA_CIRCLE_O : ICON_FA_CIRCLE;
            if (node->Name.empty())
                snprintf(name, sizeof(name), "%s%p", type, node.get());
            else
                snprintf(name, sizeof(name), "%s%s", type, node->Name.c_str());
            ImGui::Selectable(name, selectedLeft == node);

            // Hovered
            if (ImGui::IsItemHovered())
            {
                hovered = node;

                // Left button
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    selectedLeft = node;
                    Inspector::Select(node);
                    Scene::Select(node);
                    clickedLeft = true;
                }
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    node->Flags &= ~TEST_OPEN_FLAG;
                    if (opened == false && node->GetChildCount() != 0)
                        node->Flags |= TEST_OPEN_FLAG;
                }

                // Right button
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    selectedRight = node;
                    clickedRight = true;
                }

                // Drag
                dragFile(node);
            }

            // Traversal
            if (opened)
            {
                ImGui::Indent();
                for (xxNodePtr const& child : *node)
                    traversal(child);
                ImGui::Unindent();
            }
        };
        if (root)
        {
            for (xxNodePtr const& child : *root)
                traversal(child);

            // Drag
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x && avail.y)
            {
                ImGui::InvisibleButton("", avail);
                dragFile(root);
            }
        }

        // Left
        if (clickedLeft == false && selectedLeft && ImGui::IsWindowHovered())
        {
            xxNodePtr const& parent = selectedLeft->GetParent();
            if (parent)
            {
                size_t index = parent->GetChildCount();
                for (size_t i = 0; i < parent->GetChildCount(); ++i)
                {
                    if (parent->GetChild(i) == selectedLeft)
                    {
                        index = i;
                        break;
                    }
                }

                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                {
                    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
                        index--;
                    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                        index++;
                    if (index < parent->GetChildCount())
                    {
                        update = true;
                        selectedLeft = parent->GetChild(index);
                        Inspector::Select(selectedLeft);
                        Scene::Select(selectedLeft);
                    }
                }
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                selectedLeft = nullptr;
                Inspector::Select(nullptr);
                Scene::Select(nullptr);
            }
        }

        // Right
        if (clickedRight == false && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        {
            selectedRight = root;
            clickedRight = true;
        }

        // Popup
        if (clickedRight)
        {
            ImGui::OpenPopup("node");
        }
        if (selectedRight && ImGui::BeginPopup("node"))
        {
            if (selectedRight->Name.empty())
                ImGui::Text("%p", selectedRight.get());
            else
                ImGui::Text("%s", selectedRight->Name.c_str());
            ImGui::Separator();
            if (selectedRight && (selectedRight == root || MiniGUI::Window::Cast(selectedRight) != nullptr) && ImGui::Button("Add Camera"))
            {
                update = true;
                xxNodePtr node = xxNode::Create();
                node->Camera = xxCamera::Create();
                node->Camera->Up = xxVector3::Z;
                node->Camera->Right = -xxVector3::X;
                node->Camera->Direction = xxVector3::Y;
                node->Camera->SetFOV(16.0f / 9.0f, 60.0f, 10000.0f);
                selectedRight->AttachChild(node);
                selectedRight->Flags |= TEST_OPEN_FLAG;
                selectedRight = nullptr;
            }
#if HAVE_MINIGUI
            if (selectedRight && (selectedRight == root || MiniGUI::Window::Cast(selectedRight) == nullptr) && ImGui::Button("Add Node"))
#else
            if (selectedRight && ImGui::Button("Add Node"))
#endif
            {
                update = true;
                selectedRight->AttachChild(xxNode::Create());
                selectedRight->Flags |= TEST_OPEN_FLAG;
                selectedRight = nullptr;
            }
#if HAVE_MINIGUI
            if (selectedRight && (selectedRight == root || MiniGUI::Window::Cast(selectedRight) != nullptr) && ImGui::Button("Add Window"))
            {
                update = true;
                selectedRight->AttachChild(MiniGUI::Window::Create());
                selectedRight->Flags |= TEST_OPEN_FLAG;
                selectedRight = nullptr;
            }
#endif
            if (selectedRight && selectedRight->GetParent() && ImGui::Button("Remove Node"))
            {
                update = true;
                if (selectedLeft == selectedRight)
                {
                    selectedLeft = nullptr;
                    Inspector::Select(nullptr);
                    Scene::Select(nullptr);
                }
                selectedRight->GetParent()->DetachChild(selectedRight);
                selectedRight = nullptr;
            }
            ImGui::Separator();
            if (ImGui::Button("Import Object"))
            {
                update = true;
                importNode = selectedRight;
                selectedRight = nullptr;
            }
            if (ImGui::Button("Export Object"))
            {
                update = true;
                exportNode = selectedRight;
                selectedRight = nullptr;
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();

#if HAVE_MINIGUI
    auto& window = MiniGUI::Window::Cast(selectedLeft);
    if (window)
    {
        MiniGUI::WindowPtr const& parent = NodeTools::GetRoot(window);
        MiniGUI::Window::Traversal(parent, [&](MiniGUI::WindowPtr const& window)
        {
            if (window != selectedLeft)
                Tools::Rect(window->GetWorldOffset(), window->GetWorldOffset() + window->GetWorldScale(), 0xFF3F3F3F);
            return true;
        });
        Tools::Rect(window->GetWorldOffset(), window->GetWorldOffset() + window->GetWorldScale());
    }
#endif
    if (hovered)
    {
#if HAVE_MINIGUI
        auto& window = MiniGUI::Window::Cast(hovered);
        if (window)
        {
            Tools::Rect(window->GetWorldOffset(), window->GetWorldOffset() + window->GetWorldScale());
        }
        else
#endif
        {
            Tools::Sphere(hovered->WorldBound.xyz, hovered->WorldBound.w);
        }
    }

    Import(updateData);
    Export(updateData);

    return update;
}
//==============================================================================
