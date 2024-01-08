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
#include "Import/Import.h"
#include "Object/Camera.h"
#include "Utility/Grid.h"
#include "Window/Hierarchy.h"
#include "Window/Inspector.h"
#include "Window/Log.h"

#define MODULE_NAME     "Editor"
#define MODULE_MAJOR    1
#define MODULE_MINOR    0

static Camera* camera;
static xxNodePtr grid;
static xxNodePtr root;
//------------------------------------------------------------------------------
moduleAPI const char* Create(const CreateData& createData)
{
    camera = Camera::CreateCamera();
    camera->GetCamera()->Location = (xxVector3::Y * -100 + xxVector3::Z * 100);
    camera->GetCamera()->LookAt(xxVector3::ZERO, xxVector3::Z);
    camera->GetCamera()->Update();

    camera->GetCamera()->LightColor = {1.0f, 0.5f, 0.5f};
    camera->GetCamera()->LightDirection = -xxVector3::Y;

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

    Camera::DestroyCamera(camera);
    camera = nullptr;
    grid = nullptr;
    root = nullptr;
}
//------------------------------------------------------------------------------
moduleAPI void Message(const MessageData& messageData)
{
    if (messageData.length == 1)
    {
        switch (xxHash(messageData.data[0]))
        {
        case xxHash("INIT"):
            grid = Grid::Create(xxVector3::ZERO, {10000, 10000});
            break;
        case xxHash("SHUTDOWN"):
        {
            grid = nullptr;
            xxNode::Traversal([](xxNodePtr const& node)
            {
                node->Invalidate();
                for (xxImagePtr const& image : node->Images)
                    image->Invalidate();
                if (node->Mesh)
                    node->Mesh->Invalidate();
                if (node->Material)
                    node->Material->Invalidate();
                return true;
            }, root);
            break;
        }
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
    bool updated = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(MODULE_NAME))
        {
            ImGui::MenuItem("About " MODULE_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &showInspector);
            ImGui::MenuItem("Log", nullptr, &showLog);
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

    updated |= Hierarchy::Update(updateData, showHierarchy, root);
    updated |= Inspector::Update(updateData, showInspector, camera->GetCamera());
    updated |= Log::Update(updateData, showLog);

    if (camera)
    {
        ImGuiIO& io = ImGui::GetIO();

        float forward_backward = 0;
        float left_right = 0;
        float speed = 10;

        if (io.WantCaptureMouse == false)
        {
            if (io.KeysDown[ImGuiKey_LeftShift])
            {
                speed = 50;
            }
            if (io.KeysDown[ImGuiKey_A])
            {
                left_right = -speed;
            }
            if (io.KeysDown[ImGuiKey_D])
            {
                left_right = speed;
            }
            if (io.KeysDown[ImGuiKey_S])
            {
                forward_backward = -speed;
            }
            if (io.KeysDown[ImGuiKey_W])
            {
                forward_backward = speed;
            }
        }

        static xxVector2 mouse = xxVector2::ZERO;
        float x = 0.0f;
        float y = 0.0f;
        if (io.MouseDown[ImGuiMouseButton_Right])
        {
            x = (io.MousePos.x - mouse.x) / 1000.0f;
            y = (io.MousePos.y - mouse.y) / 1000.0f * (16.0f / 9.0f);
        }
        mouse.x = io.MousePos.x;
        mouse.y = io.MousePos.y;

        camera->SetFOV(updateData.width / (float)updateData.height, 60.0f, 10000.0f);
        if (forward_backward || left_right || x || y)
        {
            camera->Update(updateData.elapsed, forward_backward, left_right, x, y);
            updated = true;
        }
    }

    if (root)
    {
        root->Update(updateData.time);
    }

    return updated;
}
//------------------------------------------------------------------------------
moduleAPI void Render(const RenderData& renderData)
{
    xxDrawData data;
    data.device = renderData.device;
    data.commandEncoder = renderData.commandEncoder;
    data.camera = camera->GetCamera().get();

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
