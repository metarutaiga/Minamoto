//==============================================================================
// Minamoto : Scene Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <xxGraphicPlus/xxCamera.h>
#include <xxGraphicPlus/xxMaterial.h>
#include <xxGraphicPlus/xxMesh.h>
#include <xxGraphicPlus/xxModifier.h>
#include <xxGraphicPlus/xxNode.h>
#include <xxGraphicPlus/xxTexture.h>
#include <Tools/CameraTools.h>
#include <Tools/NodeTools.h>
#if HAVE_MINIGUI
#include <MiniGUI/Window.h>
#endif
#include "Utility/Grid.h"
#include "Utility/Tools.h"
#include "Hierarchy.h"
#include "Inspector.h"
#include "Profiler.h"
#include "Scene.h"

//==============================================================================
xxCameraPtr Scene::screenCamera;
xxCameraPtr Scene::sceneCamera;
xxNodePtr Scene::sceneRoot;
xxNodePtr Scene::selected;
//------------------------------------------------------------------------------
static xxVector3 sceneArcball = {0.85f, -M_PI_2, 14.0f};
static xxVector3 sceneCameraOffset = {1.0f, 0.0f, 0.0f};
static xxNodePtr sceneGrid;
static xxDrawData sceneDrawData;
static xxVector2 viewPos;
static xxVector2 viewSize;
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
            for (xxTexturePtr const& texture : node->Material->Textures)
            {
                if (texture->Path.empty() == false)
                    texture->Initialize(0, 0, 0, 0, 0, 0);
                texture->Invalidate();
            }
        }
        return true;
    };

    xxNode::Traversal(sceneRoot, invalidate);
    xxNode::Traversal(sceneGrid, invalidate);

    if (suspend)
        return;

    screenCamera = nullptr;
    sceneCamera = nullptr;
    sceneRoot = nullptr;
    sceneGrid = nullptr;
    sceneDrawData = xxDrawData{};
    selected = nullptr;
    viewViewport = nullptr;
}
//------------------------------------------------------------------------------
void Scene::Select(xxNodePtr const& node)
{
    selected = node;
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
void Scene::DrawCameraLine(const UpdateData& updateData, xxCameraPtr const& camera)
{
    if (camera)
    {
        float frustumNear = camera->FrustumNear;
        float frustumFar = std::min(camera->FrustumFar, 10.0f);
        xxVector3 const& location = camera->Location;
        xxVector3 near[4] =
        {
            { 0.0f, 0.0f, frustumNear },
            { 0.0f, 1.0f, frustumNear },
            { 1.0f, 1.0f, frustumNear },
            { 1.0f, 0.0f, frustumNear },
        };
        xxVector3 far[4] =
        {
            { 0.0f, 0.0f, frustumFar },
            { 0.0f, 1.0f, frustumFar },
            { 1.0f, 1.0f, frustumFar },
            { 1.0f, 0.0f, frustumFar },
        };
        for (int i = 0; i < 4; ++i)
        {
            near[i] = CameraTools::GetScreenPosToWorldPos(camera, near[i]);
            far[i] = CameraTools::GetScreenPosToWorldPos(camera, far[i]);
        }
        for (int i = 0; i < 4; ++i)
        {
            Tools::Line(location, near[i]);
            Tools::Line(near[i], far[i]);
            Tools::Line(near[i], near[(i + 1) % 4]);
            Tools::Line(far[i], far[(i + 1) % 4]);
        }
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
#if HAVE_MINIGUI
static void MiniGUIEditor(MiniGUI::WindowPtr const& window)
{
    if (window == nullptr)
        return;

    static int dragType = 0;
    static bool dragging = false;
    constexpr int LT = 0;
    constexpr int RT = 1;
    constexpr int LB = 2;
    constexpr int RB = 3;

    xxVector2 mousePos;
    mousePos.x = ImGui::GetIO().MousePos.x;
    mousePos.y = ImGui::GetIO().MousePos.y;

    // Drag
    if (dragging)
    {
        xxVector2 lt = window->GetOffset();
        xxVector2 rb = window->GetOffset() + window->GetScale();
        xxVector2 pt = (mousePos - viewPos) / viewSize;
        MiniGUI::WindowPtr parent = window->GetParent();
        if (parent)
        {
            pt = (pt - parent->GetWorldOffset()) / parent->GetWorldScale();
        }
        switch (dragType)
        {
        case LT:
            window->SetScale({ rb.x - pt.x, rb.y - pt.y});
            window->SetOffset({ pt.x, pt.y });
            break;
        case RT:
            window->SetScale({ pt.x - lt.x, rb.y - pt.y});
            window->SetOffset({ lt.x, pt.y });
            break;
        case LB:
            window->SetScale({ rb.x - pt.x, pt.y - lt.y});
            window->SetOffset({ pt.x, lt.y });
            break;
        case RB:
            window->SetScale({ pt.x - lt.x, pt.y - lt.y});
            window->SetOffset({ lt.x, lt.y });
            break;
        default:
            break;
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            dragging = false;
        }
        return;
    }

    // Edge
    xxVector2 lt = window->GetWorldOffset();
    xxVector2 rb = window->GetWorldOffset() + window->GetWorldScale();
    lt = lt * viewSize + viewPos;
    rb = rb * viewSize + viewPos;
    bool hitX0 = std::fabs(mousePos.x - lt.x) < 8.0f;
    bool hitY0 = std::fabs(mousePos.y - lt.y) < 8.0f;
    bool hitX1 = std::fabs(mousePos.x - rb.x) < 8.0f;
    bool hitY1 = std::fabs(mousePos.y - rb.y) < 8.0f;
    if (hitX0 || hitX1 || hitY0 || hitY1)
    {
        int type = -1;
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        if (hitX0 && hitY0)
        {
            type = LT;
            drawList->AddCircle({ lt.x, lt.y }, 8.0f, 0xFFFFFFFF);
        }
        else if (hitX1 && hitY0)
        {
            type = RT;
            drawList->AddCircle({ rb.x, lt.y }, 8.0f, 0xFFFFFFFF);
        }
        else if (hitX0 && hitY1)
        {
            type = LB;
            drawList->AddCircle({ lt.x, rb.y }, 8.0f, 0xFFFFFFFF);
        }
        else if (hitX1 && hitY1)
        {
            type = RB;
            drawList->AddCircle({ rb.x, rb.y }, 8.0f, 0xFFFFFFFF);
        }
        if (type >= LT && type <= RB)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                dragType = type;
                dragging = true;
                return;
            }
        }
    }

    // Hit
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        MiniGUI::WindowPtr root = NodeTools::GetRoot(window);
        MiniGUI::WindowPtr selected = window;
        auto callback = [&](MiniGUI::WindowPtr const& window)
        {
            xxVector2 lt = window->GetWorldOffset();
            xxVector2 rb = window->GetWorldOffset() + window->GetWorldScale();
            lt = lt * viewSize + viewPos;
            rb = rb * viewSize + viewPos;
            if (mousePos.x >= lt.x && mousePos.x <= rb.x && mousePos.y >= lt.y && mousePos.y <= rb.y)
            {
                Hierarchy::Select(window);
                Inspector::Select(window);
                Scene::Select(window);
            }
            return true;
        };
        MiniGUI::Window::Traversal(root, callback);
    }
}
#endif
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
        struct
        {
            size_t bone;
            size_t nodeTotal;
            size_t nodeActive;
            size_t modifierTotal;
            size_t modifierActive;
        } Count = {};
        auto callback = [&](xxNodePtr const& node)
        {
            node->Flags &= ~NodeTools::TEST_CHECK_FLAG;
            for (auto const& data : node->Bones)
            {
                if (data.bone.use_count())
                {
                    xxNodePtr const& bone = (xxNodePtr&)data.bone;
                    if ((bone->Flags & NodeTools::TEST_CHECK_FLAG) == 0)
                    {
                        bone->Flags |= NodeTools::TEST_CHECK_FLAG;
                        Count.bone++;
                    }
                }
            }
            Count.nodeTotal++;
            Count.modifierTotal += node->Modifiers.size();
            if ((node->Flags & xxNode::UPDATE_SKIP) == 0)
            {
                Count.nodeActive++;
                Count.modifierActive += node->Modifiers.size();
            }
            return true;
        };
        for (xxNodePtr const& node : (*sceneRoot))
        {
#if HAVE_MINIGUI
            if (MiniGUI::Window::Cast(node))
                continue;
#endif
            xxNode::Traversal(node, callback);
        }
        Profiler::Count(xxHash("Bone Count"), Count.bone);
        Profiler::Count(xxHash("Node Total Count"), Count.nodeTotal);
        Profiler::Count(xxHash("Node Active Count"), Count.nodeActive);
        Profiler::Count(xxHash("Modifier Total Count"), Count.modifierTotal);
        Profiler::Count(xxHash("Modifier Active Count"), Count.modifierActive);
        updated |= Count.modifierTotal != 0;

        // Scene
        Profiler::Begin(xxHash("Scene Update"));
        for (xxNodePtr const& node : (*sceneRoot))
        {
#if HAVE_MINIGUI
            if (MiniGUI::Window::Cast(node))
                continue;
#endif
            node->Update(updateData.time);
        }
        sceneRoot->UpdateBound();
        Profiler::End(xxHash("Scene Update"));

        // MiniGUI
        Profiler::Begin(xxHash("MiniGUI Update"));
        for (xxNodePtr const& node : (*sceneRoot))
        {
#if HAVE_MINIGUI
            auto& window = MiniGUI::Window::Cast(node);
            if (window)
            {
                MiniGUI::Window::Update(window, updateData.time, viewSize);
            }
#endif
        }
        Profiler::End(xxHash("MiniGUI Update"));

        if (show)
        {
            DrawBoneLine(sceneRoot);
            DrawNodeLine(sceneRoot);
            DrawNodeBound(sceneRoot);

            if (selected)
            {
                DrawCameraLine(updateData, selected->Camera);
            }
        }
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
        if (ImGui::IsWindowFocused())
        {
            if (ImGui::IsKeyDown(ImGuiKey_Escape))
            {
                Hierarchy::Select(nullptr);
                Inspector::Select(nullptr);
                Scene::Select(nullptr);
            }
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
            if (ImGui::IsKeyPressed(ImGuiKey_Z) && sceneCamera)
            {
                if (selected)
                {
                    Tools::LookAtFromBound(sceneCamera, selected->WorldBound, xxVector3::Z);
                }
                else if (sceneRoot)
                {
                    Tools::LookAtFromBound(sceneCamera, sceneRoot->WorldBound, xxVector3::Z);
                }
            }
        }

        static xxVector2 mouse = xxVector2::ZERO;
        float x = 0.0f;
        float y = 0.0f;
        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsWindowHovered())
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
                // ImGui::GetCurrentWindow() failed on MSVC
                ImGuiContext* context = ImGui::GetCurrentContext();
                ImGui::FocusWindow(context->CurrentWindow);
                x = (io.MousePos.x - mouse.x) / 1000.0f;
                y = (io.MousePos.y - mouse.y) / 1000.0f * (16.0f / 9.0f);
            }
        }
        mouse.x = io.MousePos.x;
        mouse.y = io.MousePos.y;

        if (forward_backward || left_right || x || y)
        {
            CameraTools::MoveCamera(sceneCamera, updateData.elapsed, forward_backward, left_right, 0.0f, x, y);
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

#if HAVE_MINIGUI
        MiniGUIEditor(MiniGUI::Window::Cast(selected));
#endif

        Tools::Draw(sceneCamera, viewSize, viewPos);
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

    sceneCamera->SetFOV(viewport_width / viewport_height, 60.0f, 10000.0f);
    sceneCamera->Update();

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

    xxNode::Traversal(sceneGrid, callback);

    Profiler::Begin(xxHash("Scene Render"));
    for (xxNodePtr const& node : (*sceneRoot))
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
    Profiler::End(xxHash("Scene Render"));
}
//------------------------------------------------------------------------------
