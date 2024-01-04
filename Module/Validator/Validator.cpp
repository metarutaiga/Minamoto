//==============================================================================
// Minamoto : Validator Source
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>

#include <xxGraphic/utility/xxNode.h>

#if DirectXMath
#include "DirectXMath.h"
using namespace DirectX;
#endif

#define PLUGIN_NAME     "Validator"
#define PLUGIN_MAJOR    1
#define PLUGIN_MINOR    0

static void ValidateNode(float time, char* text, size_t count);

//------------------------------------------------------------------------------
moduleAPI const char* Create(const CreateData& createData)
{
    return PLUGIN_NAME;
}
//------------------------------------------------------------------------------
moduleAPI void Shutdown(const ShutdownData& shutdownData)
{

}
//------------------------------------------------------------------------------
moduleAPI bool Update(const UpdateData& updateData)
{
    static bool showNode = false;
    static bool showAbout = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(PLUGIN_NAME))
        {
            ImGui::MenuItem("Validate Node", nullptr, &showNode);
            ImGui::Separator();
            ImGui::MenuItem("About " PLUGIN_NAME, nullptr, &showAbout);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showNode)
    {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Validate Node", &showNode))
        {
            static char text[4096];
            ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);

            if (ImGui::Button("Validate"))
            {
                ValidateNode(updateData.time, text, sizeof(text));
            }

            ImGui::End();
        }
    }

    if (showAbout)
    {
        if (ImGui::Begin("About " PLUGIN_NAME, &showAbout))
        {
            ImGui::Text("%s Plugin Version %d.%d", PLUGIN_NAME, PLUGIN_MAJOR, PLUGIN_MINOR);
            ImGui::Text("Build Date : %s %s", __DATE__, __TIME__);
            ImGui::Separator();
            ImGui::DumpBuildInformation();
            ImGui::End();
        }
    }

    return false;
}
//------------------------------------------------------------------------------
moduleAPI void Render(const RenderData& renderData)
{

}
//------------------------------------------------------------------------------
void ValidateNode(float time, char* text, size_t count)
{
    int step = 0;

    // 1. Create Root
    xxNodePtr root = xxNode::Create();
    step += snprintf(text + step, count - step, "Root : %p\n", root.get());

    // 2. Create Child
    xxNodePtr child = xxNode::Create();
    step += snprintf(text + step, count - step, "Child : %p\n", child.get());

    // 3. Attach Child
    bool attachChild = root->AttachChild(child);
    step += snprintf(text + step, count - step, "Attach Child : %s\n", attachChild ? "TRUE" : "FALSE");

    // 4. Get Children Count
    step += snprintf(text + step, count - step, "Root Children Count : %zu\n", root->GetChildCount());
    for (size_t i = 0; i < root->GetChildCount(); ++i)
    {
        xxNodePtr node = root->GetChild(i);
        if (node != nullptr)
        {
            step += snprintf(text + step, count - step, "Children (%zu) : %p\n", i, node.get());
        }
    }

    // 5. Create GrandChild
    xxNodePtr grandChild = xxNode::Create();
    step += snprintf(text + step, count - step, "Child : %p\n", grandChild.get());

    // 6. Attach GrandChild
    bool attachGrandChild = child->AttachChild(grandChild);
    step += snprintf(text + step, count - step, "Attach Child : %s\n", attachGrandChild ? "TRUE" : "FALSE");

    // 7. Get Child's Children Count
    step += snprintf(text + step, count - step, "Child's Children Count : %zu\n", child->GetChildCount());
    for (size_t i = 0; i < child->GetChildCount(); ++i)
    {
        xxNodePtr node = child->GetChild(i);
        if (node != nullptr)
        {
            step += snprintf(text + step, count - step, "Children (%zu) : %p\n", i, node.get());
        }
    }

    // 8. Set Local Matrix
    root->LocalMatrix = { xxVector4::Z, -xxVector4::Y, xxVector4::X, xxVector4::WHITE };
    child->LocalMatrix = { xxVector4::Y, -xxVector4::X, xxVector4::Z, xxVector4::WHITE };
    child->SetRotate({ xxVector3::Y, -xxVector3::X, xxVector3::Z });
    child->SetTranslate(xxVector3::WHITE);
    child->SetScale(2.0f);
    child->UpdateRotateTranslateScale();
    grandChild->LocalMatrix = { xxVector4::X, -xxVector4::Z, xxVector4::Y, xxVector4::WHITE };
    step += snprintf(text + step, count - step, "SetLocalMatrix\n");

    // 9. Update
    root->Update(time);
    step += snprintf(text + step, count - step, "Update Root : %f\n", time);

    // 10. Get Root World Matrix
    for (int i = 0; i < 4; ++i)
    {
        const xxMatrix4& worldMatrix = root->WorldMatrix;
        step += snprintf(text + step, count - step, "Root Matrix (%u) : %f %f %f %f\n", i, worldMatrix.v[i].x, worldMatrix.v[i].y, worldMatrix.v[i].z, worldMatrix.v[i].w);
    }

    // 11. Get Child World Matrix
    for (int i = 0; i < 4; ++i)
    {
        const xxMatrix4& worldMatrix = child->WorldMatrix;
        step += snprintf(text + step, count - step, "Child Matrix (%u) : %f %f %f %f\n", i, worldMatrix.v[i].x, worldMatrix.v[i].y, worldMatrix.v[i].z, worldMatrix.v[i].w);
    }

    // 12. Get GrandChild World Matrix
    for (int i = 0; i < 4; ++i)
    {
        const xxMatrix4& worldMatrix = grandChild->WorldMatrix;
        step += snprintf(text + step, count - step, "GrandChild Matrix (%u) : %f %f %f %f\n", i, worldMatrix.v[i].x, worldMatrix.v[i].y, worldMatrix.v[i].z, worldMatrix.v[i].w);
    }

    // 13. xxMatrix Multiply
    xxMatrix4x4 a = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    xxMatrix4x4 b = { 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 4 };
    xxMatrix4x4 c = a * b;
    for (int i = 0; i < 4; ++i)
    {
        step += snprintf(text + step, count - step, "Matrix Multiply (%u) : %f %f %f %f\n", i, c.v[i].x, c.v[i].y, c.v[i].z, c.v[i].w);
    }
#if DirectXMath
    XMMATRIX aa = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    XMMATRIX bb = { 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 4 };
    XMMATRIX cc = aa * bb;
    for (int i = 0; i < 4; ++i)
    {
        step += snprintf(text + step, count - step, "Matrix Multiply (%u) : %f %f %f %f\n", i, cc.r[i][0], cc.r[i][1], cc.r[i][2], cc.r[i][3]);
    }
#endif
}
//------------------------------------------------------------------------------
