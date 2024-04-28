//==============================================================================
// Minamoto : Editor Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <utility/xxImage.h>
#include <utility/xxMaterial.h>
#include <utility/xxMesh.h>
#include <utility/xxNode.h>
#include <Runtime/Runtime.h>
#include "Graphic/ShaderDisassembly.h"
#include "Import/Import.h"
#include "Object/Camera.h"
#include "Utility/Grid.h"
#include "Utility/Tools.h"
#include "Window/Hierarchy.h"
#include "Window/Inspector.h"
#include "Window/Log.h"

#define MODULE_NAME     "Editor"
#define MODULE_MAJOR    1
#define MODULE_MINOR    0

static Camera* editorCamera;
static xxNodePtr grid;
static xxNodePtr root;
static size_t modifierCount;
//------------------------------------------------------------------------------
moduleAPI const char* Create(const CreateData& createData)
{
    Runtime::Initialize();
    ShaderDisassembly::Initialize();

    editorCamera = Camera::CreateCamera();
    xxCameraPtr const& camera = editorCamera->GetCamera();
    camera->Location = (xxVector3::Y * -100 + xxVector3::Z * 100);
    camera->LookAt(xxVector3::ZERO, xxVector3::Z);
    camera->Update();

    camera->LightColor = {1.0f, 0.5f, 0.5f};
    camera->LightDirection = -xxVector3::Y;

    grid = Grid::Create(xxVector3::ZERO, {10000, 10000});
    root = xxNode::Create();

    Import::Initialize();
    Hierarchy::Initialize();
    Inspector::Initialize();
    Log::Initialize();

    return MODULE_NAME;
}
//------------------------------------------------------------------------------
moduleAPI void Shutdown(const ShutdownData& shutdownData)
{
    Import::Shutdown();
    Hierarchy::Shutdown();
    Inspector::Shutdown();
    Log::Shutdown();

    Camera::DestroyCamera(editorCamera);
    editorCamera = nullptr;
    grid = nullptr;
    root = nullptr;

    ShaderDisassembly::Shutdown();
    Runtime::Shutdown();
}
//------------------------------------------------------------------------------
moduleAPI void Message(const MessageData& messageData)
{
    if (messageData.length == 1)
    {
        switch (xxHash(messageData.data[0]))
        {
        case xxHash("INIT"):
            Runtime::Initialize();
            ShaderDisassembly::Initialize();
            grid = Grid::Create(xxVector3::ZERO, {10000, 10000});
            break;
        case xxHash("SHUTDOWN"):
            grid = nullptr;
            xxNode::Traversal([](xxNodePtr const& node)
            {
                node->Invalidate();
                if (node->Mesh)
                    node->Mesh->Invalidate();
                if (node->Material)
                {
                    node->Material->Invalidate();
                    for (xxImagePtr const& image : node->Material->Images)
                        image->Invalidate();
                }
                return true;
            }, root);
            ShaderDisassembly::Shutdown();
            Runtime::Shutdown();
            break;
        default:
            break;
        }
    }
}
//------------------------------------------------------------------------------
moduleAPI bool Update(const UpdateData& updateData)
{
    static bool showAbout = false;
    static bool showHierarchy = true;
    static bool showInspector = true;
    static bool showLog = true;
    static bool showShaderDisassembly = false;
    float menuBarHeight = 0.0f;
    bool updated = false;

    if (ImGui::BeginMainMenuBar())
    {
        menuBarHeight = ImGui::GetWindowHeight();

        if (ImGui::BeginMenu(MODULE_NAME))
        {
            ImGui::MenuItem("About " MODULE_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &showInspector);
            ImGui::MenuItem("Log", nullptr, &showLog);
            ImGui::Separator();
            ImGui::MenuItem("Shader Disassembly", nullptr, &showShaderDisassembly);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showAbout)
    {
        if (ImGui::Begin("About " MODULE_NAME, &showAbout))
        {
            ImGui::Text("%s Module Version %d.%d", MODULE_NAME, MODULE_MAJOR, MODULE_MINOR);
            ImGui::Text("Build Date : %s %s", __DATE__, __TIME__);
            ImGui::Separator();
            ImGui::DumpBuildInformation();
            ImGui::End();
        }
    }

    if (editorCamera)
    {
        ImGuiIO& io = ImGui::GetIO();

        float forward_backward = 0;
        float left_right = 0;
        float speed = 10;

        if (io.WantCaptureMouse == false)
        {
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
            {
                speed = 50;
            }
            if (ImGui::IsKeyDown(ImGuiKey_A))
            {
                left_right = -speed;
            }
            if (ImGui::IsKeyDown(ImGuiKey_D))
            {
                left_right = speed;
            }
            if (ImGui::IsKeyDown(ImGuiKey_S))
            {
                forward_backward = -speed;
            }
            if (ImGui::IsKeyDown(ImGuiKey_W))
            {
                forward_backward = speed;
            }
        }

        static xxVector2 mouse = xxVector2::ZERO;
        float x = 0.0f;
        float y = 0.0f;
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            x = (io.MousePos.x - mouse.x) / 1000.0f;
            y = (io.MousePos.y - mouse.y) / 1000.0f * (16.0f / 9.0f);
        }
        mouse.x = io.MousePos.x;
        mouse.y = io.MousePos.y;

        editorCamera->SetFOV(updateData.width / (float)updateData.height, 60.0f, 10000.0f);
        if (forward_backward || left_right || x || y)
        {
            editorCamera->Update(updateData.elapsed, forward_backward, left_right, x, y);
            updated = true;
        }
    }

    if (root)
    {
        root->Update(updateData.time);

        modifierCount = 0;
        xxNode::Traversal([&](xxNodePtr const& node)
        {
            modifierCount += node->Modifiers.size();
            return true;
        }, root);
    }

    updated |= Hierarchy::Update(updateData, menuBarHeight, showHierarchy, root, editorCamera->GetCamera());
    updated |= Inspector::Update(updateData, menuBarHeight, showInspector, editorCamera->GetCamera());
    updated |= Log::Update(updateData, showLog);
    updated |= ShaderDisassembly::Update(updateData, showShaderDisassembly);
    updated |= modifierCount != 0;

    Tools::Draw(editorCamera->GetCamera());

    return updated;
}
//------------------------------------------------------------------------------
moduleAPI void Render(const RenderData& renderData)
{
    xxDrawData data;
    data.device = renderData.device;
    data.commandEncoder = renderData.commandEncoder;
    data.camera = editorCamera->GetCamera().get();

    xxNode::Traversal([&](xxNodePtr const& node)
    {
        node->Draw(data);
        return true;
    }, root);

    if (grid)
    {
        grid->Draw(data);
    }
}
//---------------------------------------------------------------------------
