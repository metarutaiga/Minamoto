//==============================================================================
// Minamoto : Inspector Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include <utility/xxCamera.h>
#include <utility/xxImage.h>
#include <utility/xxMaterial.h>
#include <utility/xxMesh.h>
#include <utility/xxNode.h>
#include <Runtime/Modifier/Modifier.h>
#include "Utility/Tools.h"
#include "ImGuiHelper.h"
#include "Log.h"
#include "Inspector.h"

float Inspector::windowWidth = 256.0f;
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

    if (ImGui::Begin(ICON_FA_INFO_CIRCLE "Inspector", &show))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
        windowWidth = ImGui::GetWindowWidth();

        // Camera
        if (camera && selected == nullptr)
        {
            UpdateCamera(updateData, camera);
        }

        // Nodes
        if (selected)
        {
            UpdateNode(updateData, selected);
            if (selected->Material)
            {
                UpdateMaterial(updateData, selected->Material);
            }
            if (selected->Mesh)
            {
                UpdateMesh(updateData, selected->Mesh);
            }
            UpdateModifier(updateData, selected->Modifiers);
        }
        ImGui::PopStyleVar();
    }
    ImGui::End();

    return false;
}
//------------------------------------------------------------------------------
#define Q "##" xxStringify(__LINE__)
//------------------------------------------------------------------------------
void Inspector::UpdateCamera(const UpdateData& updateData, xxCameraPtr const& camera)
{
    if (ImGui::CollapsingHeader(ICON_FA_CAMERA "Camera" Q, nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat3("Right" Q, camera->Right.Array(), -1.0f, 1.0f);
        ImGui::SliderFloat3("Up" Q, camera->Up.Array(), -1.0f, 1.0f);
        ImGui::SliderFloat3("Direction" Q, camera->Direction.Array(), -1.0f, 1.0f);
        ImGui::InputFloat3("Location" Q, camera->Location.Array());
    }
    if (ImGui::CollapsingHeader(ICON_FA_LIGHTBULB_O "Light" Q, nullptr, ImGuiTreeNodeFlags_DefaultOpen))
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
    if (ImGui::CollapsingHeader(ICON_FA_CUBE "Node" Q, nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    {
        char name[64];
        strcpy(name, selected->Name.c_str());
        if (ImGui::InputText("Name" Q, name, 64))
            selected->Name = name;
        ImGui::SliderFloat3("Local" Q, selected->LocalMatrix.v[0].Array(), -1.0f, 1.0f);
        ImGui::SliderFloat3("" Q, selected->LocalMatrix.v[1].Array(), -1.0f, 1.0f);
        ImGui::SliderFloat3("" Q, selected->LocalMatrix.v[2].Array(), -1.0f, 1.0f);
        ImGui::InputFloat3("" Q, selected->LocalMatrix.v[3].Array());
        ImGui::SliderFloat3("World" Q, selected->WorldMatrix.v[0].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
        ImGui::SliderFloat3("" Q, selected->WorldMatrix.v[1].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
        ImGui::SliderFloat3("" Q, selected->WorldMatrix.v[2].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
        ImGui::InputFloat3("" Q, selected->WorldMatrix.v[3].Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat3("Bound" Q, selected->WorldBound.Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat("" Q, &selected->WorldBound.w, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
        if (node->Bones.empty() == false)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, 0);
            bool open = ImGui::CollapsingHeader(ICON_FA_STREET_VIEW "Bones" Q, nullptr, ImGuiTreeNodeFlags_Framed);
            ImGui::PopStyleColor();
            if (open)
            {
                static int current = 0;
                static unsigned int hovered = 0; hovered = UINT_MAX;

                ImGui::ListBox("Bones" Q, &current, [](void* user_data, int idx)
                {
                    if (hovered == UINT_MAX && ImGui::IsItemHovered())
                        hovered = idx - 1;
                    auto& bones = *(std::vector<xxNode::BoneData>*)user_data;
                    xxNodePtr bone = bones[idx].bone.lock();
                    return bone ? bone->Name.c_str() : "(nullptr)";
                }, &node->Bones, (int)node->Bones.size());
                if (hovered == UINT_MAX && ImGui::IsItemHovered())
                    hovered = (int)node->Bones.size() - 1;

                if (hovered < node->Bones.size())
                {
                    auto& boneData = node->Bones[hovered];
                    if (boneData.bone.expired() == false)
                    {
                        auto const& bone = (xxNodePtr&)boneData.bone;
                        if (ImGui::BeginTooltip())
                        {
                            ImGui::InputText("Name" Q, bone->Name.data(), 64, ImGuiInputTextFlags_ReadOnly);
                            ImGui::SliderFloat3("World" Q, bone->WorldMatrix.v[0].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::SliderFloat3("" Q, bone->WorldMatrix.v[1].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::SliderFloat3("" Q, bone->WorldMatrix.v[2].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::InputFloat3("" Q, bone->WorldMatrix.v[3].Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
                            ImGui::SliderFloat3("Skin" Q, boneData.skinMatrix.v[0].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::SliderFloat3("" Q, boneData.skinMatrix.v[1].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::SliderFloat3("" Q, boneData.skinMatrix.v[2].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::InputFloat3("" Q, boneData.skinMatrix.v[3].Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
                            ImGui::SliderFloat3("Bone" Q, boneData.boneMatrix.v[0].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::SliderFloat3("" Q, boneData.boneMatrix.v[1].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::SliderFloat3("" Q, boneData.boneMatrix.v[2].Array(), -1.0f, 1.0f, "%.3f", ImGuiSliderFlags_NoInput);
                            ImGui::InputFloat3("" Q, boneData.boneMatrix.v[3].Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputFloat3("Bound" Q, boneData.bound.Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
                            ImGui::InputFloat("" Q, &boneData.bound.w, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
                            ImGui::EndTooltip();
                        }
                        xxNodePtr const& parent = bone->GetParent();
                        if (parent)
                        {
                            Tools::Line(parent->WorldMatrix.v[3].xyz, bone->WorldMatrix.v[3].xyz);
                        }
                    }
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void Inspector::UpdateMaterial(const UpdateData& updateData, xxMaterialPtr const& material)
{
    xxMaterialPtr invalidate;

    ImGui::Separator();
    if (ImGui::CollapsingHeader(ICON_FA_ADJUST "Material" Q, nullptr, ImGuiTreeNodeFlags_DefaultOpen))
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
        for (auto& image : material->Images)
        {
            strcpy(name, ICON_FA_PICTURE_O);
            strcat(name, image->Name.c_str());
            ImGui::PushStyleColor(ImGuiCol_Header, 0);
            bool open = ImGui::CollapsingHeader(name, nullptr, ImGuiTreeNodeFlags_None);
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
            {
                uint64_t texture = image->GetTexture();
                if (texture && ImGui::BeginTooltip())
                {
                    ImGui::Image(texture, ImVec2(256, 256));
                    ImGui::EndTooltip();
                }
            }
            if (open)
            {
                ImGui::InputInt("Width" Q, (int*)&image->Width, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Height" Q, (int*)&image->Height, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Depth" Q, (int*)&image->Depth, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Mipmap" Q, (int*)&image->Mipmap, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Array" Q, (int*)&image->Array, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::Separator();
                ImGui::Checkbox("Clamp U" Q, &image->ClampU);
                ImGui::Checkbox("Clamp V" Q, &image->ClampV);
                ImGui::Checkbox("Clamp W" Q, &image->ClampW);
                ImGui::Separator();
                ImGui::Checkbox("Linear MAG" Q, &image->FilterMag);
                ImGui::Checkbox("Linear MIN" Q, &image->FilterMin);
                ImGui::Checkbox("Linear MIP" Q, &image->FilterMip);
                ImGui::SliderChar("Anisotropic" Q, &image->Anisotropic, 1, 16);
            }
        }
        UpdateModifier(updateData, material->Modifiers);
    }

    // Material Invalidate
    if (invalidate)
    {
        invalidate->Invalidate();

        xxNodePtr root = selected;
        while (xxNodePtr parent = root->GetParent())
            root = parent;

        xxNode::Traversal([invalidate](xxNodePtr const& node)
        {
            if (node->Material == invalidate)
                node->Invalidate();
            return true;
        }, root);
    }
}
//------------------------------------------------------------------------------
void Inspector::UpdateMesh(const UpdateData& updateData, xxMeshPtr const& mesh)
{
    ImGui::Separator();
    if (ImGui::CollapsingHeader(ICON_FA_CUBES "Mesh" Q, nullptr, ImGuiTreeNodeFlags_None))
    {
        char name[64];
        strcpy(name, mesh->Name.c_str());
        if (ImGui::InputText("Name" Q, name, 64))
            mesh->Name = name;
        ImGui::InputChar("Color" Q, (char*)&mesh->ColorCount, 0, 0, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputChar("Normal" Q, (char*)&mesh->NormalCount, 0, 0, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputChar("Texture" Q, (char*)&mesh->TextureCount, 0, 0, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Stride" Q, (int*)&mesh->Stride, 0, 0, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Vertex" Q, (int*)&mesh->VertexCount, 0, 0, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputInt("Index" Q, (int*)&mesh->IndexCount, 0, 0, ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat3("Bound" Q, (float*)mesh->Bound.Array(), "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat("" Q, (float*)&mesh->Bound.w, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
    }
}
//------------------------------------------------------------------------------
void Inspector::UpdateModifier(const UpdateData& updateData, std::vector<xxModifierData> const& modifierData)
{
    if (modifierData.empty())
        return;
    if (ImGui::CollapsingHeader(ICON_FA_FILM "Modifier" Q, nullptr, ImGuiTreeNodeFlags_None))
    {
        for (auto const& data : modifierData)
        {
            ImGui::PushStyleColor(ImGuiCol_Header, 0);
            bool open = ImGui::CollapsingHeader(Modifier::Name(*data.modifier).c_str(), nullptr, ImGuiTreeNodeFlags_None);
            ImGui::PopStyleColor();
            if (open)
            {
                size_t size = data.modifier->Data.size();
                size_t count = Modifier::Count(*data.modifier);
                ImGui::InputInt("Size" Q, (int*)&size, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Count" Q, (int*)&count, 0, 0, ImGuiInputTextFlags_ReadOnly);
                ImGui::InputFloat("Time" Q, (float*)&data.time, 0, 0, "%.3f", ImGuiInputTextFlags_ReadOnly);
                ImGui::InputInt("Index" Q, (int*)&data.index, 0, 0, ImGuiInputTextFlags_ReadOnly);
            }
        }
    }
}
//==============================================================================
