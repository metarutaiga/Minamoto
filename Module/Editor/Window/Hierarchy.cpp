//==============================================================================
// Minamoto : Hierarchy Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <utility/xxBinary.h>
#include <utility/xxNode.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include "Import/ImportWavefront.h"
#include "Hierarchy.h"
#include "Inspector.h"

#if defined(__APPLE__)
#define HAVE_FILEDIALOG 1
#elif defined(_WIN32) && !defined(__clang__)
#define HAVE_FILEDIALOG 1
#else
#define HAVE_FILEDIALOG 0
#endif

xxNodePtr Hierarchy::selectedLeft;
xxNodePtr Hierarchy::selectedRight;
xxNodePtr Hierarchy::importNode;
ImGuiFileDialog* Hierarchy::fileDialog;
//==============================================================================
void Hierarchy::Initialize()
{
    selectedLeft = nullptr;
    selectedRight = nullptr;
    importNode = nullptr;
#if HAVE_FILEDIALOG
    fileDialog = new ImGuiFileDialog;
#endif
}
//------------------------------------------------------------------------------
void Hierarchy::Shutdown()
{
#if HAVE_FILEDIALOG
    delete fileDialog;
#endif
    selectedLeft = nullptr;
    selectedRight = nullptr;
    importNode = nullptr;
    fileDialog = nullptr;
}
//------------------------------------------------------------------------------
bool Hierarchy::Update(const UpdateData& updateData, bool& show, xxNodePtr const& root)
{
    if (show == false)
        return false;

    bool update = false;
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float windowHeight = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;
    float borderHeight = style.FramePadding.y * 2.0f;
    float windowWidth = 256.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + windowHeight));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, viewport->Size.y - windowHeight * 2.0f - borderHeight * 2.0f));
    if (ImGui::Begin("Hierarchy", &show))
    {
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

            // Left button
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                selectedLeft = node;
                Inspector::Select(node);
                clickedLeft = true;
            }

            // Right button
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                selectedRight = node;
                clickedRight = true;
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
            for (size_t i = 0; i < root->GetChildCount(); ++i)
                traversal(root->GetChild(i));
        }

        // Left
        if (clickedLeft == false && selectedLeft && ImGui::IsWindowHovered())
        {
            xxNodePtr parent = selectedLeft->GetParent();
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
            if (selectedRight && selectedRight->GetParent() && selectedRight->GetChildCount() == 0 && ImGui::Button("Remove Node"))
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
                selectedRight = nullptr;
            }
            if (ImGui::Button("Import Wavefront"))
            {
                update = true;
                importNode = selectedRight;
                selectedRight = nullptr;
#if HAVE_FILEDIALOG
                IGFD::FileDialogConfig config =
                {
                    .path = xxGetDocumentPath(),
                };
                fileDialog->OpenDialog("Import Wavefront", "Choose File", ".obj", config);
#else
                importNode->AttachChild(ImportWavefront::CreateObject("Miku/HatsuneMiku.obj"));
#endif
            }
            ImGui::Separator();
            if (ImGui::Button("Export Object"))
            {
                update = true;
                selectedRight = nullptr;
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();

#if HAVE_FILEDIALOG
    if (importNode && fileDialog->Display("Import Wavefront"))
    {
        if (fileDialog->IsOk())
        {
            std::string name = fileDialog->GetFilePathName();
            importNode->AttachChild(ImportWavefront::CreateObject(name.c_str()));
        }
        importNode = nullptr;
        fileDialog->Close();
    }
#endif

    return update;
}
//==============================================================================
