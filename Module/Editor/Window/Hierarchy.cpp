//==============================================================================
// Minamoto : Hierarchy Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <utility/xxCamera.h>
#include <utility/xxFile.h>
#include <utility/xxNode.h>
#include <Graphic/Binary.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include "Import/ImportFBX.h"
#include "Import/ImportWavefront.h"
#include "Utility/Tools.h"
#include "Hierarchy.h"
#include "Log.h"
#include "Inspector.h"

#if defined(__APPLE__)
#define HAVE_FILEDIALOG 1
#elif defined(_WIN32) && !defined(__clang__)
#define HAVE_FILEDIALOG 1
#else
#define HAVE_FILEDIALOG 0
#endif

float Hierarchy::windowWidth = 256.0f;
xxNodePtr Hierarchy::selectedLeft;
xxNodePtr Hierarchy::selectedRight;
xxNodePtr Hierarchy::importNode;
xxNodePtr Hierarchy::exportNode;
char Hierarchy::importName[1024];
char Hierarchy::exportName[1024];
ImGuiFileDialog* Hierarchy::importFileDialog;
ImGuiFileDialog* Hierarchy::exportFileDialog;
bool Hierarchy::drawNodeLine = false;
bool Hierarchy::drawNodeBound = false;
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
            const char* filters =
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
        ImGui::Checkbox("Optimize Mesh", &Import::EnableOptimizeMesh);
        ImGui::Checkbox("Texture Flip V", &Import::EnableTextureFlipV);
        if (ImGui::Button("Import"))
        {
            float begin = xxGetCurrentTime();
            xxNodePtr node;
            if (strstr(importName, ".fbx"))
                node = ImportFBX::Create(importName);
            if (strstr(importName, ".obj"))
                node = ImportWavefront::Create(importName);
            if (strstr(importName, ".xxb"))
                node = Binary::Load(importName);
            if (node)
            {
                float time = xxGetCurrentTime() - begin;
                xxLog("Hierarchy", "Import : %s (%0.fus)", xxFile::GetName(importName).c_str(), time * 1000000);
                if (Import::EnableMergeNode)
                {
                    Import::MergeNode(importNode, node, importNode);
                }
                else
                {
                    importNode->AttachChild(node);
                }

                xxNode* root = importNode.get();
                while (xxNode* parent = root->GetParent().get())
                    root = parent;
                root->CreateLinearMatrix();

                importNode = nullptr;
                show = false;
            }
        }
        ImGui::End();
    }

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
            const char* filters =
                "Double Cross Binary(*.xxb){.xxb},"
                "All Files(*.*){.*},";
            exportFileDialog->OpenDialog("Export", "Choose File", filters, config);
#endif
        }
        if (ImGui::Button("Export"))
        {
            float begin = xxGetCurrentTime();
            if (Binary::Save(exportName, exportNode))
            {
                float time = xxGetCurrentTime() - begin;
                xxLog("Hierarchy", "Export : %s (%0.fus)", xxFile::GetName(exportName).c_str(), time * 1000000);
                exportNode = nullptr;
                show = false;
            }
        }
        ImGui::End();
    }

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
void Hierarchy::Option(const UpdateData& updateData, float menuBarHeight, xxNodePtr const& root, xxCameraPtr const& camera)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + windowWidth, viewport->Pos.y + menuBarHeight));
    if (ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground))
    {
        ImGui::Checkbox("Node Line", &drawNodeLine);
        ImGui::Checkbox("Node Bound", &drawNodeBound);
        ImGui::End();
    }
    if (drawNodeLine)
    {
        xxNode::Traversal([&](xxNodePtr const& node)
        {
            xxNodePtr const& parent = node->GetParent();
            if (parent)
            {
                Tools::Line(parent->WorldMatrix[3].xyz, node->WorldMatrix[3].xyz);
            }
            return true;
        }, root);
    }
    if (drawNodeBound)
    {
        xxNode::Traversal([&](xxNodePtr const& node)
        {
            xxVector4 const& bound = node->WorldBound;
            if (bound.w != 0.0f)
            {
                Tools::Sphere(bound.xyz, bound.w);
            }
            return true;
        }, root);
    }
}
//------------------------------------------------------------------------------
bool Hierarchy::Update(const UpdateData& updateData, float menuBarHeight, bool& show, xxNodePtr const& root, xxCameraPtr const& camera)
{
    if (show == false)
        return false;

    bool update = false;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, viewport->Size.y - menuBarHeight - Log::GetWindowHeight()));

    if (ImGui::Begin("Hierarchy", &show))
    {
        windowWidth = ImGui::GetWindowWidth();

        bool clickedLeft = false;
        bool clickedRight = false;
        std::function<void(xxNodePtr const&)> traversal = [&](xxNodePtr const& node)
        {
            if (node == nullptr)
                return;

            ImGuiTreeNodeFlags flags = node->GetChildCount() ? 0 : ImGuiTreeNodeFlags_Leaf;
            if (node == selectedLeft || node == selectedRight)
                flags |= ImGuiTreeNodeFlags_Selected;

            bool opened;
            if (node->Name.empty())
                opened = ImGui::TreeNodeEx(node.get(), flags, "%p", node.get());
            else
                opened = ImGui::TreeNodeEx(node.get(), flags, "%s", node->Name.c_str());

            // Hovered
            if (ImGui::IsItemHovered())
            {
                Tools::Sphere(node->WorldBound.xyz, node->WorldBound.w);

                // Left button
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    selectedLeft = node;
                    Inspector::Select(node);
                    clickedLeft = true;
                }

                // Right button
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    selectedRight = node;
                    clickedRight = true;
                }
            }

            // Traversal
            if (opened)
            {
                for (size_t i = 0; i < node->GetChildCount(); ++i)
                    traversal(node->GetChild(i));
                ImGui::TreePop();
            }
        };
        if (root)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
            for (size_t i = 0; i < root->GetChildCount(); ++i)
                traversal(root->GetChild(i));
            ImGui::PopStyleVar();
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
                    }
                }
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                selectedLeft = nullptr;
                Inspector::Select(nullptr);
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
            if (ImGui::Button("Add Node"))
            {
                update = true;
                selectedRight->AttachChild(xxNode::Create());
                selectedRight = nullptr;
            }
            if (selectedRight && selectedRight->GetParent() && ImGui::Button("Remove Node"))
            {
                update = true;
                if (selectedLeft == selectedRight)
                {
                    selectedLeft = nullptr;
                    Inspector::Select(selectedRight);
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
            ImGui::Separator();
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

    Import(updateData);
    Export(updateData);
    Option(updateData, menuBarHeight, root, camera);

    return update;
}
//==============================================================================
