//==============================================================================
// Minamoto : Scene Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <xxGraphic.h>
#include <utility/xxCamera.h>
#include <utility/xxMaterial.h>
#include <utility/xxNode.h>
#include "Object/Camera.h"
#include "Utility/Grid.h"
#include "Utility/Tools.h"
#include "Scene.h"

//==============================================================================
static xxNodePtr grid;
static xxNodePtr sceneRoot;
static xxDrawData sceneDrawData;
static ImVec2 viewPos;
static ImVec2 viewSize;
static ImGuiViewport* viewViewport;
//------------------------------------------------------------------------------
void Scene::Initialize()
{
    grid = Grid::Create(xxVector3::ZERO, {10000, 10000});
}
//------------------------------------------------------------------------------
void Scene::Shutdown()
{
    grid = nullptr;
    sceneRoot = nullptr;
    sceneDrawData = xxDrawData{};
    viewViewport = nullptr;
}
//------------------------------------------------------------------------------
bool Scene::Update(const UpdateData& updateData, bool& show, xxNodePtr const& root, Camera* camera)
{
    if (show == false)
        return false;

    bool updated = false;
    if (ImGui::Begin("Scene", &show))
    {
        if (camera == nullptr)
        {
            ImGui::End();
            return false;
        }

        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        ImVec2 cursor = ImGui::GetCursorPos();
        ImVec2 framePadding = ImGui::GetStyle().FramePadding;
        sceneRoot = root;
        sceneDrawData = xxDrawData{updateData.device, 0, camera->GetCamera().get()};
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
        float speed = 10.0f;

        if (ImGui::IsWindowHovered())
        {
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
            {
                speed = 50.0f;
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
        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            x = (io.MousePos.x - mouse.x) / 1000.0f;
            y = (io.MousePos.y - mouse.y) / 1000.0f * (16.0f / 9.0f);
        }
        mouse.x = io.MousePos.x;
        mouse.y = io.MousePos.y;

        if (forward_backward || left_right || x || y)
        {
            camera->Update(updateData.elapsed, forward_backward, left_right, x, y);
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
        ImGui::ColorButton("", ImVec4(0.45f, 0.55f, 0.60f, 1.00f), flags, viewSize);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddCallback(Callback, nullptr);
        drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

        Tools::Draw(camera->GetCamera());

        ImGui::End();
    }

    return updated;
}
//------------------------------------------------------------------------------
void Scene::Callback(const ImDrawList* list, const ImDrawCmd* cmd)
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float dpiScale = 0.0f;
    uint64_t commandEncoder = 0;
    ImGuiPlatformIO& io = ImGui::GetPlatformIO();
    for (int i = 0; i < io.Viewports.size(); ++i)
    {
        ImGuiViewport* viewport = io.Viewports[i];
        if (viewport == nullptr || viewport != viewViewport || viewport->RendererUserData == nullptr)
            continue;
        x = viewport->Pos.x;
        y = viewport->Pos.y;
        width = viewport->Size.x;
        height = viewport->Size.y;
        dpiScale = viewport->DpiScale;
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

    xxSetScissor(commandEncoder, viewport_x, viewport_y, viewport_width, viewport_height);
    sceneDrawData.commandEncoder = commandEncoder;
    sceneDrawData.camera->SetFOV(viewport_width / viewport_height, 60.0f, 10000.0f);
    sceneDrawData.camera->SetViewportMatrix(width * dpiScale, height * dpiScale, viewport_x, viewport_y, viewport_width, viewport_height);
    sceneDrawData.camera->Update();

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

    xxNode::Traversal(callback, sceneRoot);
    xxNode::Traversal(callback, grid);
}
//------------------------------------------------------------------------------
