//==============================================================================
// Minamoto : Inspector Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <utility/xxCamera.h>
#include <utility/xxMaterial.h>
#include <utility/xxMesh.h>
#include <utility/xxNode.h>
#include "Inspector.h"

xxNodePtr Inspector::selected;
//==============================================================================
void Inspector::Initialize()
{
    selected = nullptr;
}
//------------------------------------------------------------------------------
void Inspector::Shutdown()
{
    selected = nullptr;
}
//------------------------------------------------------------------------------
void Inspector::Select(xxNodePtr const& node)
{
    selected = node;
}
//------------------------------------------------------------------------------
bool Inspector::Update(const UpdateData& updateData, bool& show, xxCameraPtr const& camera)
{
    if (show == false)
        return false;

    bool update = false;
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    float windowHeight = ImGui::GetFontSize() + style.FramePadding.y * 2.0f;
    float borderHeight = style.FramePadding.y * 2.0f;
    float windowWidth = 256.0f;

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - windowWidth, viewport->Pos.y + windowHeight));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, viewport->Size.y - windowHeight * 2.0f - borderHeight * 2.0f));
    if (ImGui::Begin("Inspector", &show))
    {
        char name[64];
        xxMaterial* materialInvalidate = nullptr;

        // Camera
        if (camera && selected == nullptr)
        {
            if (ImGui::CollapsingHeader("Camera", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::SliderFloat3("Right##1", camera->Right.Array(), -1.0f, 1.0f);
                ImGui::SliderFloat3("Up##2", camera->Up.Array(), -1.0f, 1.0f);
                ImGui::SliderFloat3("Direction##3", camera->Direction.Array(), -1.0f, 1.0f);
                ImGui::InputFloat3("Location##4", camera->Location.Array());
            }
            if (ImGui::CollapsingHeader("Light", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::ColorEdit3("Color##11", camera->LightColor.Array());
                ImGui::SliderFloat3("Direction##12", camera->LightDirection.Array(), -1.0f, 1.0f);
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    camera->LightDirection /= camera->LightDirection.Length();
            }
        }

        // Nodes
        if (selected)
        {
            if (ImGui::CollapsingHeader("Node", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
            {
                strcpy(name, selected->Name.c_str());
                if (ImGui::InputText("Name##20", name, 64))
                    selected->Name = name;
                ImGui::SliderFloat3("Local##21", selected->LocalMatrix.v[0].Array(), -1.0f, 1.0f);
                ImGui::SliderFloat3("##22", selected->LocalMatrix.v[1].Array(), -1.0f, 1.0f);
                ImGui::SliderFloat3("##23", selected->LocalMatrix.v[2].Array(), -1.0f, 1.0f);
                ImGui::InputFloat3("##24", selected->LocalMatrix.v[3].Array());
                ImGui::SliderFloat3("World##21", selected->WorldMatrix.v[0].Array(), -1.0f, 1.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
                ImGui::SliderFloat3("##22", selected->WorldMatrix.v[1].Array(), -1.0f, 1.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
                ImGui::SliderFloat3("##23", selected->WorldMatrix.v[2].Array(), -1.0f, 1.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
                ImGui::InputFloat3("##24", selected->WorldMatrix.v[3].Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
            }
            xxMesh* mesh = selected->Mesh.get();
            if (mesh && ImGui::CollapsingHeader("Mesh", nullptr, ImGuiTreeNodeFlags_None))
            {
                strcpy(name, mesh->Name.c_str());
                if (ImGui::InputText("Name##30", name, 64))
                    mesh->Name = name;
                int vertexCount = mesh->GetVertexCount();
                int indexCount = mesh->GetIndexCount();
                ImGui::InputInt("Vertex##31", &vertexCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Index##32", &indexCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Stride##33", (int*)&mesh->Stride, 1, 100, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Color##34", (int*)&mesh->ColorCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Normal##35", (int*)&mesh->NormalCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Texture##36", (int*)&mesh->TextureCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
            }
            xxMaterial* material = selected->Material.get();
            if (material && ImGui::CollapsingHeader("Material", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
            {
                strcpy(name, material->Name.c_str());
                if (ImGui::InputText("Name##40", name, 64))
                    material->Name = name;
                if (ImGui::Checkbox("Lighting", &material->Lighting))
                    materialInvalidate = material;
                if (material->Lighting)
                {
                    ImGui::ColorEdit3("Ambient##41", material->AmbientColor.Array());
                    ImGui::ColorEdit3("Diffuse##42", material->DiffuseColor.Array());
                    ImGui::ColorEdit3("Emissive##43", material->EmissiveColor.Array());
                    if (ImGui::Checkbox("Specular", &material->Specular))
                        materialInvalidate = material;
                    if (material->Specular)
                    {
                        ImGui::ColorEdit3("Specular##44", material->SpecularColor.Array());
                        ImGui::SliderFloat("Power##45", &material->SpecularHighlight, 0.0f, 256.0f);
                    }
                }
            }
        }

        // Material Invalidate
        if (materialInvalidate)
        {
            materialInvalidate->Invalidate();

            xxNodePtr parent = selected;
            while (parent->GetParent())
                parent = parent->GetParent();

            std::function<void(xxNodePtr const&)> traversal = [&](xxNodePtr const& node)
            {
                if (node == nullptr)
                    return;

                if (node->Material.get() == materialInvalidate)
                    node->Invalidate();

                for (size_t i = 0; i < node->GetChildCount(); ++i)
                    traversal(node->GetChild(i));
            };
            traversal(parent);
        }
    }
    ImGui::End();

    return update;
}
//==============================================================================
