//==============================================================================
// Minamoto : Scene Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <xxGraphic.h>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include <utility/xxCamera.h>
#include <utility/xxImage.h>
#include <utility/xxMaterial.h>
#include <utility/xxMesh.h>
#include <utility/xxModifier.h>
#include <utility/xxNode.h>
#include <Tools/CameraTools.h>
#include <Tools/NodeTools.h>
#include "Utility/Grid.h"
#include "Utility/Tools.h"
#include "Profiler.h"
#include "Scene.h"

//==============================================================================
xxCameraPtr Scene::sceneCamera;
xxNodePtr Scene::sceneRoot;
//------------------------------------------------------------------------------
static xxVector3 sceneArcball = {0.85f, -M_PI_2, 14.0f};
static xxVector3 sceneCameraOffset = {1.0f, 0.0f, 0.0f};
static xxNodePtr sceneGrid;
static xxDrawData sceneDrawData;
static ImVec2 viewPos;
static ImVec2 viewSize;
static ImGuiViewport* viewViewport;
static bool drawBoneLine = false;
static bool drawNodeLine = false;
static bool drawNodeBound = false;
//------------------------------------------------------------------------------
void Scene::Initialize()
{
    if (sceneCamera == nullptr)
    {
        sceneCamera = xxCamera::Create();
        CameraTools::MoveArcball(sceneCameraOffset, sceneArcball, 0, 0);
        sceneCamera->Location = (xxVector3::Y * -10 + xxVector3::Z * 10);
        sceneCamera->LookAt(xxVector3::ZERO, xxVector3::Z);
        sceneCamera->Update();

        sceneCamera->LightColor = {1.0f, 0.5f, 0.5f};
        sceneCamera->LightDirection = -xxVector3::Y;
    }

    if (sceneRoot == nullptr)
        sceneRoot = xxNode::Create();
    if (sceneGrid == nullptr)
        sceneGrid = Grid::Create(xxVector3::ZERO, {10000, 10000});
}
//------------------------------------------------------------------------------
void Scene::Shutdown(bool suspend)
{
    auto invalidate = [](xxNodePtr const& node)
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
    };

    xxNode::Traversal(sceneRoot, invalidate);
    xxNode::Traversal(sceneGrid, invalidate);

    if (suspend)
        return;

    sceneCamera = nullptr;
    sceneRoot = nullptr;
    sceneGrid = nullptr;
    sceneDrawData = xxDrawData{};
    viewViewport = nullptr;
}
//------------------------------------------------------------------------------
void Scene::DrawBoneLine(xxNodePtr const& root)
{
    if (drawBoneLine)
    {
        xxNode::Traversal(root, [&](xxNodePtr const& node)
        {
            for (auto const& data : node->Bones)
            {
                if (data.bone.use_count())
                {
                    xxNodePtr const& bone = (xxNodePtr&)data.bone;
                    xxNodePtr const& parent = bone->GetParent();
                    if (parent)
                    {
                        Tools::Line(parent->GetWorldTranslate(), bone->GetWorldTranslate());
                    }
                }
            }
            return true;
        });
    }
}
//------------------------------------------------------------------------------
void Scene::DrawNodeLine(xxNodePtr const& root)
{
    if (drawNodeLine)
    {
        xxNode::Traversal(root, [&](xxNodePtr const& node)
        {
            xxNodePtr const& parent = node->GetParent();
            if (parent)
            {
                Tools::Line(parent->GetWorldTranslate(), node->GetWorldTranslate());
            }
            return true;
        });
    }
}
//------------------------------------------------------------------------------
void Scene::DrawNodeBound(xxNodePtr const& root)
{
    if (drawNodeBound)
    {
        xxNode::Traversal(root, [&](xxNodePtr const& node)
        {
            xxVector4 const& bound = node->WorldBound;
            if (bound.w != 0.0f)
            {
                Tools::Sphere(bound.xyz, bound.w);
            }
            return true;
        });
    }
}
//------------------------------------------------------------------------------
bool Scene::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    bool updated = false;
    if (sceneRoot)
    {
        NodeTools::UpdateNodeFlags(sceneRoot);

        // Count
        size_t boneCount = 0;
        size_t nodeTotalCount = 0;
        size_t nodeActiveCount = 0;
        size_t modifierTotalCount = 0;
        size_t modifierActiveCount = 0;
        xxNode::Traversal(sceneRoot, [&](xxNodePtr const& node)
        {
            size_t TEMP_FLAG = ~(SIZE_MAX >> 1);
            node->Flags &= ~TEMP_FLAG;
            for (auto const& data : node->Bones)
            {
                if (data.bone.use_count())
                {
                    xxNodePtr const& bone = (xxNodePtr&)data.bone;
                    if ((bone->Flags & TEMP_FLAG) == 0)
                    {
                        bone->Flags |= TEMP_FLAG;
                        boneCount++;
                    }
                }
            }
            nodeTotalCount++;
            modifierTotalCount += node->Modifiers.size();
            if ((node->Flags & xxNode::UPDATE_SKIP) == 0)
            {
                nodeActiveCount++;
                modifierActiveCount += node->Modifiers.size();
            }
            return true;
        });
        xxNode::Traversal(sceneRoot, [&](xxNodePtr const& node)
        {
            size_t TEMP_FLAG = ~(SIZE_MAX >> 1);
            node->Flags &= ~TEMP_FLAG;
            return true;
        });
        Profiler::Count(xxHash("Bone Count"), boneCount);
        Profiler::Count(xxHash("Node Total Count"), nodeTotalCount);
        Profiler::Count(xxHash("Node Active Count"), nodeActiveCount);
        Profiler::Count(xxHash("Modifier Total Count"), modifierTotalCount);
        Profiler::Count(xxHash("Modifier Active Count"), modifierActiveCount);
        updated |= modifierTotalCount != 0;

        // Scene
        Profiler::Begin(xxHash("Scene Update"));
        sceneRoot->Update(updateData.time);
        Profiler::End(xxHash("Scene Update"));

        DrawBoneLine(sceneRoot);
        DrawNodeLine(sceneRoot);
        DrawNodeBound(sceneRoot);
    }

    if (ImGui::Begin(ICON_FA_GLOBE "Scene", &show))
    {
        ImGui::Checkbox("##1", &drawBoneLine);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", "Draw Bone Line");
        }
        ImGui::SameLine();
        ImGui::Checkbox("##2", &drawNodeLine);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", "Draw Node Line");
        }
        ImGui::SameLine();
        ImGui::Checkbox("##3", &drawNodeBound);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", "Draw Node Bound");
        }

        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        ImVec2 cursor = ImGui::GetCursorPos();
        ImVec2 framePadding = ImGui::GetStyle().FramePadding;
        sceneDrawData = xxDrawData{updateData.device, 0, sceneCamera.get()};
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
            if (ImGui::IsKeyPressed(ImGuiKey_Z) && sceneCamera && sceneRoot)
            {
                Tools::LookAtFromBound(sceneCamera, sceneRoot->WorldBound, xxVector3::Z);
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
            CameraTools::MoveCamera(sceneCamera, updateData.elapsed, forward_backward, left_right, x, y);
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

        Tools::Draw(sceneCamera);
    }
    ImGui::End();

    return updated;
}
//------------------------------------------------------------------------------
void Scene::Callback(const ImDrawList* list, const ImDrawCmd* cmd)
{
    if (viewViewport == nullptr || viewViewport->RendererUserData == nullptr)
        return;

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
        if (viewport != viewViewport)
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

    sceneCamera->SetFOV(viewport_width / viewport_height, 60.0f, 10000.0f);
    sceneCamera->Update();
    CameraTools::SetViewport(sceneCamera, width * dpiScale, height * dpiScale, viewport_x, viewport_y, viewport_width, viewport_height);

    xxSetScissor(commandEncoder, viewport_x, viewport_y, viewport_width, viewport_height);
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

    Profiler::Begin(xxHash("Scene Render"));
    xxNode::Traversal(sceneRoot, callback);
    Profiler::End(xxHash("Scene Render"));

    xxNode::Traversal(sceneGrid, callback);
}
//------------------------------------------------------------------------------
