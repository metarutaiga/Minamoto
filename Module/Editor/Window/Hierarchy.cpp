//==============================================================================
// Minamoto : Hierarchy Source
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <utility/xxNode.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include "Import/ImportWavefront.h"
#include "Hierarchy.h"

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
        std::function<void(xxNodePtr const&)> traversal = [&](xxNodePtr const& node)
        {
            if (node == nullptr)
                return;

            ImGuiTreeNodeFlags flags = node->GetChildCount() ? 0 : ImGuiTreeNodeFlags_Leaf;
            if (node == selectedLeft || node == selectedRight)
                flags |= ImGuiTreeNodeFlags_Selected;
            bool opened = ImGui::TreeNodeEx(node.get(), flags, "%p %s", node.get(), node->Name.c_str());

            // Left button
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                selectedLeft = node;
            }

            // Right button
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                selectedRight = node;
                ImGui::OpenPopup("node");
            }

            // Popup
            if (selectedRight == node)
            {
                if (ImGui::BeginPopup("node"))
                {
                    ImGui::Text("%p", node.get());
                    ImGui::Separator();
                    if (ImGui::Button("Add Node"))
                    {
                        update = true;
                        node->AttachChild(xxNode::Create());
                        selectedRight = nullptr;
                    }
                    if (node->GetParent() && node->GetChildCount() == 0 && ImGui::Button("Remove Node"))
                    {
                        update = true;
                        if (selectedLeft == selectedRight)
                            selectedLeft = nullptr;
                        selectedRight = nullptr;
                        node->GetParent()->DetachChild(node);
                    }
                    ImGui::Separator();
                    if (ImGui::Button("Import Wavefront"))
                    {
                        update = true;
                        importNode = selectedRight;
                        selectedRight = nullptr;
#if HAVE_FILEDIALOG
                        fileDialog->OpenDialog("Import Wavefront", "Choose File", ".obj", ".");
#endif
                    }
                    ImGui::EndPopup();
                }
                else
                {
                    selectedRight = nullptr;
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
        traversal(root);
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
