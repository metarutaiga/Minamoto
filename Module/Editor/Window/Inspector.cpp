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
        // Camera
        if (camera && selected == nullptr)
        {
            UpdateCamera(updateData, camera);
        }

        // Nodes
        if (selected)
        {
            UpdateNode(updateData, selected);
            if (selected->Mesh)
            {
                UpdateMesh(updateData, selected->Mesh);
            }
            if (selected->Material)
            {
                UpdateMaterial(updateData, selected->Material);
            }
        }
    }
    ImGui::End();

    return update;
}
//------------------------------------------------------------------------------
#define Q "##" xxStringify(__LINE__)
//------------------------------------------------------------------------------
void Inspector::UpdateCamera(const UpdateData& updateData, xxCameraPtr const& camera)
{
    if (ImGui::CollapsingHeader("Camera" Q, nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat3("Right" Q, camera->Right.Array(), -1.0f, 1.0f);
        ImGui::SliderFloat3("Up" Q, camera->Up.Array(), -1.0f, 1.0f);
        ImGui::SliderFloat3("Direction" Q, camera->Direction.Array(), -1.0f, 1.0f);
        ImGui::InputFloat3("Location" Q, camera->Location.Array());
    }
    if (ImGui::CollapsingHeader("Light" Q, nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("Color" Q, camera->LightColor.Array());
        ImGui::SliderFloat3("Direction" Q, camera->LightDirection.Array(), -1.0f, 1.0f);
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            camera->LightDirection /= camera->LightDirection.Length();
    }
}
//------------------------------------------------------------------------------
void Inspector::UpdateNode(const UpdateData& updateData, xxNodePtr const& node)
{
    if (ImGui::CollapsingHeader("Node" Q, nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    {
        char name[64];
        strcpy(name, selected->Name.c_str());
        if (ImGui::InputText("Name" Q, name, 64))
            selected->Name = name;
        ImGui::SliderFloat3("Local" Q, selected->LocalMatrix.v[0].Array(), -1.0f, 1.0f);
        ImGui::SliderFloat3("" Q, selected->LocalMatrix.v[1].Array(), -1.0f, 1.0f);
        ImGui::SliderFloat3("" Q, selected->LocalMatrix.v[2].Array(), -1.0f, 1.0f);
        ImGui::InputFloat3("" Q, selected->LocalMatrix.v[3].Array());
        ImGui::SliderFloat3("World" Q, selected->WorldMatrix.v[0].Array(), -1.0f, 1.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::SliderFloat3("" Q, selected->WorldMatrix.v[1].Array(), -1.0f, 1.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::SliderFloat3("" Q, selected->WorldMatrix.v[2].Array(), -1.0f, 1.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat3("" Q, selected->WorldMatrix.v[3].Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
    }
}
//------------------------------------------------------------------------------
void Inspector::UpdateMaterial(const UpdateData& updateData, xxMaterialPtr const& material)
{
    xxMaterialPtr invalidate;

    if (ImGui::CollapsingHeader("Material" Q, nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    {
        char name[64];
        strcpy(name, material->Name.c_str());
        if (ImGui::InputText("Name" Q, name, 64))
            material->Name = name;
        ImGui::SliderFloat("Opacity" Q, &material->Opacity, 0.0f, 1.0f);
        if (ImGui::Checkbox("Lighting" Q, &material->Lighting))
            invalidate = material;
        if (material->Lighting)
        {
            ImGui::ColorEdit3("Ambient" Q, material->AmbientColor.Array());
            ImGui::ColorEdit3("Diffuse" Q, material->DiffuseColor.Array());
            ImGui::ColorEdit3("Emissive" Q, material->EmissiveColor.Array());
            if (ImGui::Checkbox("Specular" Q, &material->Specular))
                invalidate = material;
            if (material->Specular)
            {
                ImGui::ColorEdit3("Specular" Q, material->SpecularColor.Array());
                ImGui::SliderFloat("Power" Q, &material->SpecularHighlight, 0.0f, 256.0f);
            }
        }
        if (ImGui::Checkbox("Blending" Q, &material->Blending))
            invalidate = material;
        if (material->Blending)
        {
            static char const* const blendTypes[] =
            {
                "Zero",
                "One",
                "SrcColor",
                "InvSrcColor",
                "DestColor",
                "InvDestColor",
                "SrcAlpha",
                "InvSrcAlpha",
                "DestAlpha",
                "InvDestAlpha",
            };

            static char const* const blendOps[] =
            {
                "Add",
                "Subtract",
                "RevSubtract",
                "Min",
                "Max",
            };

            auto Combo = [&](char const* label, auto&& list, auto& value)
            {
                if (ImGui::BeginCombo(label, value.c_str()))
                {
                    for (char const* type : list)
                    {
                        if (ImGui::Selectable(type, type == value))
                        {
                            value = type;
                            invalidate = material;
                        }
                    }
                    ImGui::EndCombo();
                }
            };

            ImGui::TextUnformatted("Color");
            Combo("Source" Q, blendTypes, material->BlendSourceColor);
            Combo("Operation" Q, blendOps, material->BlendOperationColor);
            Combo("Destination" Q, blendTypes, material->BlendDestinationColor);
            ImGui::Separator();
            ImGui::TextUnformatted("Alpha");
            Combo("Source" Q, blendTypes, material->BlendSourceAlpha);
            Combo("Operation" Q, blendOps, material->BlendOperationAlpha);
            Combo("Destination" Q, blendTypes, material->BlendDestinationAlpha);
        }
    }

    // Material Invalidate
    if (invalidate)
    {
        invalidate->Invalidate();

        xxNodePtr parent = selected;
        while (parent->GetParent())
            parent = parent->GetParent();

        xxNode::Traversal([invalidate](xxNodePtr const& node)
        {
            if (node->Material == invalidate)
                node->Invalidate();
            return true;
        }, parent);
    }
}
//------------------------------------------------------------------------------
void Inspector::UpdateMesh(const UpdateData& updateData, xxMeshPtr const& mesh)
{
    if (ImGui::CollapsingHeader("Mesh" Q, nullptr, ImGuiTreeNodeFlags_None))
    {
        char name[64];
        strcpy(name, mesh->Name.c_str());
        if (ImGui::InputText("Name" Q, name, 64))
            mesh->Name = name;
        int vertexCount = mesh->GetVertexCount();
        int indexCount = mesh->GetIndexCount();
        ImGui::InputInt("Vertex" Q, &vertexCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Index" Q, &indexCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Stride" Q, (int*)&mesh->Stride, 1, 100, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Color" Q, (int*)&mesh->ColorCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Normal" Q, (int*)&mesh->NormalCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Texture" Q, (int*)&mesh->TextureCount, 1, 100, ImGuiInputTextFlags_ReadOnly);
    }
}
//==============================================================================
