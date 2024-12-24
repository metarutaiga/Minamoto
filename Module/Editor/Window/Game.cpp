//==============================================================================
// Minamoto : Game Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <xxGraphicPlus/xxCamera.h>
#include <xxGraphicPlus/xxMaterial.h>
#include <xxGraphicPlus/xxNode.h>
#include <Tools/CameraTools.h>
#if HAVE_MINIGUI
#include <MiniGUI/Window.h>
#endif
#include "Profiler.h"
#include "Scene.h"
#include "Game.h"

//==============================================================================
static xxCameraPtr screenCamera;
static xxDrawData sceneDrawData;
static xxVector2 viewPos;
static xxVector2 viewSize;
static ImGuiViewport* viewViewport;
//------------------------------------------------------------------------------
void Game::Initialize()
{
    if (screenCamera == nullptr)
    {
        screenCamera = xxCamera::Create();
        screenCamera->FrustumLeft = 0;
        screenCamera->FrustumRight = 1;
        screenCamera->FrustumTop = 0;
        screenCamera->FrustumBottom = 1;
        screenCamera->Up = xxVector3::Y;
        screenCamera->Right = xxVector3::X;
        screenCamera->Direction = xxVector3::Z;
        screenCamera->Location = xxVector3::ZERO;
        screenCamera->Update();
        screenCamera->ProjectionMatrix[3].x = -1.0f;
        screenCamera->ProjectionMatrix[3].y = 1.0f;
    }
}
//------------------------------------------------------------------------------
void Game::Shutdown(bool suspend)
{
    if (suspend)
        return;

    screenCamera = nullptr;
    sceneDrawData = xxDrawData{};
    viewViewport = nullptr;
}
//------------------------------------------------------------------------------
bool Game::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    bool updated = false;
    if (ImGui::Begin(ICON_FA_GAMEPAD "Game", &show))
    {
        xxCameraPtr camera;
        for (xxNodePtr const& node : (*Scene::sceneRoot))
        {
            if (node->Camera)
            {
                camera = node->Camera;
                break;
            }
        }

        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        ImVec2 cursor = ImGui::GetCursorPos();
        ImVec2 framePadding = ImGui::GetStyle().FramePadding;
        sceneDrawData = xxDrawData{updateData.device, 0, camera.get()};
        viewPos.x = pos.x + cursor.x;
        viewPos.y = pos.y + cursor.y;
        viewSize.x = size.x - cursor.x - framePadding.x * 2.0f;
        viewSize.y = size.y - cursor.y - framePadding.y * 3.0f;
        ImGuiViewport* viewport = ImGui::GetWindowViewport();
        if (viewViewport != viewport)
        {
            viewViewport = viewport;
            updated = true;
        }

        float forward_backward = 0.0f;
        float left_right = 0.0f;
        float up_down = 0.0f;
        float speed = 10.0f;
        if (ImGui::IsWindowFocused())
        {
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
            {
                speed = 50.0f;
            }
            if (ImGui::IsKeyDown(ImGuiKey_Q))
            {
                up_down = -speed;
            }
            if (ImGui::IsKeyDown(ImGuiKey_E))
            {
                up_down = speed;
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

        if (camera && (forward_backward || left_right || up_down))
        {
            CameraTools::MoveCamera(camera, updateData.elapsed, forward_backward, left_right, up_down, 0.0f, 0.0f);
            updated = true;
        }

        ImGuiColorEditFlags flags = 0;
        flags |= ImGuiColorEditFlags_NoAlpha;
        flags |= ImGuiColorEditFlags_NoPicker;
        flags |= ImGuiColorEditFlags_NoOptions;
        flags |= ImGuiColorEditFlags_NoSmallPreview;
        flags |= ImGuiColorEditFlags_NoInputs;
        flags |= ImGuiColorEditFlags_NoTooltip;
        flags |= ImGuiColorEditFlags_NoLabel;
        flags |= ImGuiColorEditFlags_NoSidePreview;
        flags |= ImGuiColorEditFlags_NoDragDrop;
        flags |= ImGuiColorEditFlags_NoBorder;
        ImGui::ColorButton("", ImVec4(0.45f, 0.55f, 0.60f, 1.00f), flags, { viewSize.x, viewSize.y });

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddCallback(Callback, nullptr);
        drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
    }
    ImGui::End();

    return updated;
}
//------------------------------------------------------------------------------
void Game::Callback(const ImDrawList* list, const ImDrawCmd* cmd)
{
    if (viewViewport == nullptr || viewViewport->RendererUserData == nullptr)
        return;

    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float dpiScale = 1.0f;
    uint64_t commandEncoder = 0;
    ImGuiPlatformIO& io = ImGui::GetPlatformIO();
    for (int i = 0; i < io.Viewports.size(); ++i)
    {
        ImGuiViewport* viewport = io.Viewports[i];
        if (viewport != viewViewport)
            continue;
        x = viewport->Pos.x;
        y = viewport->Pos.y;
        width = viewport->Size.x;
        height = viewport->Size.y;
#if defined(xxMACOS) || defined(xxIOS)
        dpiScale = viewport->DpiScale;
#endif
        commandEncoder = *(uint64_t*)viewport->RendererUserData;
        break;
    }
    if (commandEncoder == 0)
        return;

    float viewport_x = (viewPos.x - x);
    float viewport_y = (viewPos.y - y);
    float viewport_width = viewSize.x;
    float viewport_height = viewSize.y;
    if (viewport_x >= width || viewport_x + width < 0.0f || viewport_y >= height || viewport_y + height < 0.0f)
        return;

    viewport_x = std::max(viewport_x, 0.0f) * dpiScale;
    viewport_y = std::max(viewport_y, 0.0f) * dpiScale;
    viewport_width = std::min(viewport_width, width - viewport_x) * dpiScale;
    viewport_height = std::min(viewport_height, height - viewport_y) * dpiScale;

    if (sceneDrawData.camera)
    {
        sceneDrawData.camera->SetFOV(viewport_width / viewport_height, 60.0f, 10000.0f);
        sceneDrawData.camera->Update();
    }

    xxSetViewport(commandEncoder, int(viewport_x), int(viewport_y), int(viewport_width), int(viewport_height), 0.0f, 1.0f);
    xxSetScissor(commandEncoder, int(viewport_x), int(viewport_y), int(viewport_width), int(viewport_height));
    sceneDrawData.commandEncoder = commandEncoder;

    auto callback = [](xxNodePtr const& node)
    {
        xxMesh* mesh = node->Mesh.get();
        xxMaterial* material = node->Material.get();
        if (mesh && material)
        {
            bool scissor = material->Scissor;
            material->Scissor = true;
            node->Draw(sceneDrawData);
            material->Scissor = scissor;
        }
        return true;
    };

    Profiler::Begin(xxHash("Game Render"));
    for (xxNodePtr const& node : (*Scene::sceneRoot))
    {
#if HAVE_MINIGUI
        auto& window = MiniGUI::Window::Cast(node);
        if (window)
        {
            xxCamera* camera = sceneDrawData.camera;
            sceneDrawData.camera = screenCamera.get();
            xxNode::Traversal(node, callback);
            sceneDrawData.camera = camera;
            continue;
        }
#endif
        xxNode::Traversal(node, callback);
    }
    Profiler::End(xxHash("Game Render"));
}
//------------------------------------------------------------------------------
