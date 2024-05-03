//==============================================================================
// Minamoto : Editor Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <Interface.h>
#include <imgui/imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome4.h>
#include <Runtime/Runtime.h>
#include "Graphic/ShaderDisassembly.h"
#include "Import/Import.h"
#include "Utility/Tools.h"
#include "Window/Hierarchy.h"
#include "Window/Inspector.h"
#include "Window/Log.h"
#include "Window/LuaConsole.h"
#include "Window/Scene.h"

#define MODULE_NAME     "Editor"
#define MODULE_MAJOR    1
#define MODULE_MINOR    0

//------------------------------------------------------------------------------
moduleAPI const char* Create(const CreateData& createData)
{
    Runtime::Initialize();
    ShaderDisassembly::Initialize();

    Import::Initialize();
    Log::Initialize();
    LuaConsole::Initialize();
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
    LuaConsole::Shutdown();
    Hierarchy::Shutdown();
    Inspector::Shutdown();
    Scene::Shutdown();

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
            Scene::Shutdown(true);
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
    static bool showLua = true;
    static bool showHierarchy = true;
    static bool showInspector = true;
    static bool showScene = true;
    static bool showShaderDisassembly = false;
    bool updated = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(MODULE_NAME))
        {
            ImGui::MenuItem("About " MODULE_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem(ICON_FA_DESKTOP     "Log", nullptr, &showLog);
            ImGui::MenuItem(ICON_FA_FILE_TEXT_O "Lua Console", nullptr, &showLua);
            ImGui::MenuItem(ICON_FA_LIST        "Hierarchy", nullptr, &showHierarchy);
            ImGui::MenuItem(ICON_FA_INFO_CIRCLE "Inspector", nullptr, &showInspector);
            ImGui::MenuItem(ICON_FA_GLOBE       "Scene", nullptr, &showScene);
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

    ImGuiID id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);

    static bool dockInitialized = false;
    if (dockInitialized == false)
    {
        dockInitialized = true;

        ImGuiID right = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 1.0f / 5.0f, nullptr, &id);
        ImGuiID down = ImGui::DockBuilderSplitNode(id, ImGuiDir_Down, 1.0f / 3.0f, nullptr, &id);
        ImGuiID left = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 1.0f / 4.0f, nullptr, &id);
        ImGui::DockBuilderDockWindow(ICON_FA_DESKTOP    "Log", down);
        ImGui::DockBuilderDockWindow(ICON_FA_FILE_TEXT_O"Lua Console", down);
        ImGui::DockBuilderDockWindow(ICON_FA_LIST       "Hierarchy", left);
        ImGui::DockBuilderDockWindow(ICON_FA_INFO_CIRCLE"Inspector", right);
        ImGui::DockBuilderDockWindow(ICON_FA_GLOBE      "Scene", id);
        ImGui::DockBuilderFinish(id);
    }

    updated |= Log::Update(updateData, showLog);
    updated |= LuaConsole::Update(updateData, showLua);
    updated |= Hierarchy::Update(updateData, showHierarchy, Scene::sceneRoot);
    updated |= Inspector::Update(updateData, showInspector, Scene::sceneCamera);
    updated |= Scene::Update(updateData, showScene);
    updated |= ShaderDisassembly::Update(updateData, showShaderDisassembly);

    return updated;
}
//------------------------------------------------------------------------------
moduleAPI void Render(const RenderData& renderData)
{
}
//---------------------------------------------------------------------------
