//==============================================================================
// Minamoto : Editor Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <imgui/imgui_internal.h>
#include <utility/xxImage.h>
#include <utility/xxMaterial.h>
#include <utility/xxMesh.h>
#include <utility/xxNode.h>
#include <Runtime/Runtime.h>
#include "Graphic/ShaderDisassembly.h"
#include "Import/Import.h"
#include "Object/Camera.h"
#include "Utility/Tools.h"
#include "Window/Hierarchy.h"
#include "Window/Inspector.h"
#include "Window/Log.h"
#include "Window/Scene.h"

#define MODULE_NAME     "Editor"
#define MODULE_MAJOR    1
#define MODULE_MINOR    0

static Camera* editorCamera;
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

    root = xxNode::Create();

    Import::Initialize();
    Log::Initialize();
    Hierarchy::Initialize();
    Inspector::Initialize();
    Scene::Initialize();

    return MODULE_NAME;
}
//------------------------------------------------------------------------------
moduleAPI void Shutdown(const ShutdownData& shutdownData)
{
    Import::Shutdown();
    Log::Shutdown();
    Hierarchy::Shutdown();
    Inspector::Shutdown();
    Scene::Shutdown();

    Camera::DestroyCamera(editorCamera);
    editorCamera = nullptr;
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
            Scene::Initialize();
            break;
        case xxHash("SHUTDOWN"):
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
            Scene::Shutdown();
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
    static bool showLog = true;
    static bool showHierarchy = true;
    static bool showInspector = true;
    static bool showSceneView = true;
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
            ImGui::MenuItem("Log", nullptr, &showLog);
            ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &showInspector);
            ImGui::MenuItem("Scene View", nullptr, &showSceneView);
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

    ImGuiID id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);

    static bool dockInitialized = false;
    if (dockInitialized == false)
    {
        dockInitialized = true;

        ImGuiID down = ImGui::DockBuilderSplitNode(id, ImGuiDir_Down, 1.0f / 8.0f, nullptr, &id);
        ImGuiID left = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 1.0f / 5.0f, nullptr, &id);
        ImGuiID right = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 1.0f / 4.0f, nullptr, &id);
        ImGui::DockBuilderDockWindow("Log", down);
        ImGui::DockBuilderDockWindow("Hierarchy", left);
        ImGui::DockBuilderDockWindow("Inspector", right);
        ImGui::DockBuilderDockWindow("Scene", id);
        ImGui::DockBuilderFinish(id);
    }

    updated |= Log::Update(updateData, showLog);
    updated |= Hierarchy::Update(updateData, menuBarHeight, showHierarchy, root, editorCamera->GetCamera());
    updated |= Inspector::Update(updateData, menuBarHeight, showInspector, editorCamera->GetCamera());
    updated |= Scene::Update(updateData, showSceneView, root, editorCamera);
    updated |= ShaderDisassembly::Update(updateData, showShaderDisassembly);
    updated |= modifierCount != 0;

    return updated;
}
//------------------------------------------------------------------------------
moduleAPI void Render(const RenderData& renderData)
{
}
//---------------------------------------------------------------------------
