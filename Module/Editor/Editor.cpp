//==============================================================================
// Minamoto : Editor Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include "Graphic/ShaderAssembler.h"
#include "Graphic/ShaderDisassembler.h"
#include "Import/Import.h"
#include "Utility/Tools.h"
#include "Window/About.h"
#include "Window/Document.h"
#include "Window/Hierarchy.h"
#include "Window/Inspector.h"
#include "Window/Log.h"
#include "Window/LuaConsole.h"
#include "Window/Profiler.h"
#include "Window/Project.h"
#include "Window/QuickJSConsole.h"
#include "Window/Scene.h"
#include "Window/Game.h"
#include "Window/Setup.h"

#if defined(__ANDROID__)
extern "C" void __cxa_pure_virtual(void) {}
#endif

//------------------------------------------------------------------------------
moduleAPI char const* Create(const CreateData& createData)
{
    Runtime::Initialize();
    ShaderAssembler::Initialize();
    ShaderDisassembler::Initialize();

    Document::Initialize();
    Import::Initialize();
    Log::Initialize();
    LuaConsole::Initialize();
    QuickJSConsole::Initialize();
    Hierarchy::Initialize();
    Inspector::Initialize();
    Profiler::Initialize();
    Project::Initialize();
    Scene::Initialize();
    Game::Initialize();
    Setup::Initialize();

    return MODULE_NAME;
}
//------------------------------------------------------------------------------
moduleAPI void Shutdown(const ShutdownData& shutdownData)
{
    Document::Shutdown();
    Import::Shutdown();
    Log::Shutdown();
    LuaConsole::Shutdown();
    QuickJSConsole::Shutdown();
    Hierarchy::Shutdown();
    Inspector::Shutdown();
    Profiler::Shutdown();
    Project::Shutdown();
    Scene::Shutdown();
    Game::Shutdown();
    Setup::Shutdown();

    ShaderAssembler::Shutdown();
    ShaderDisassembler::Shutdown();
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
            ShaderDisassembler::Initialize();
            Project::Initialize();
            Scene::Initialize();
            Game::Initialize();
            break;
        case xxHash("SHUTDOWN"):
            Project::Shutdown(true);
            Scene::Shutdown(true);
            Game::Shutdown(true);
            ShaderDisassembler::Shutdown();
            Runtime::Shutdown(true);
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
    static bool showSetup = false;
    static bool showProject = true;
    static bool showLog = true;
    static bool showLuaConsole = true;
    static bool showQuickJSConsole = true;
    static bool showProfiler = true;
    static bool showHierarchy = true;
    static bool showInspector = true;
    static bool showScene = true;
    static bool showGame = true;
    static bool showShaderAssembler = false;
    static bool showShaderDisassembler = false;
    bool updated = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(MODULE_NAME))
        {
            ImGui::MenuItem(ICON_FA_QUESTION_CIRCLE "About " MODULE_NAME, nullptr, &showAbout);
            ImGui::Separator();
            ImGui::MenuItem(ICON_FA_COG             "Setup", nullptr, &showSetup);
            ImGui::Separator();
            ImGui::MenuItem(ICON_FA_BOOKMARK        "Project", nullptr, &showProject);
            ImGui::MenuItem(ICON_FA_DESKTOP         "Log", nullptr, &showLog);
            ImGui::MenuItem(ICON_FA_LAPTOP          "Lua Console", nullptr, &showLuaConsole);
            ImGui::MenuItem(ICON_FA_TABLET          "QuickJS Console", nullptr, &showQuickJSConsole);
            ImGui::MenuItem(ICON_FA_BAR_CHART       "Profiler", nullptr, &showProfiler);
            ImGui::MenuItem(ICON_FA_LIST            "Hierarchy", nullptr, &showHierarchy);
            ImGui::MenuItem(ICON_FA_INFO_CIRCLE     "Inspector", nullptr, &showInspector);
            ImGui::MenuItem(ICON_FA_GLOBE           "Scene", nullptr, &showScene);
            ImGui::MenuItem(ICON_FA_GAMEPAD         "Game", nullptr, &showGame);
            ImGui::Separator();
            ImGui::MenuItem(ICON_FA_PENCIL          "Shader Assembler", nullptr, &showShaderAssembler);
            ImGui::MenuItem(ICON_FA_FILE_TEXT       "Shader Disassembler", nullptr, &showShaderDisassembler);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGuiID id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None);

    static bool dockPreInitialized = false;
    if (dockPreInitialized == false)
    {
        dockPreInitialized = true;

        ImGuiID right = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 1.0f / 5.0f, nullptr, &id);
        ImGuiID down = ImGui::DockBuilderSplitNode(id, ImGuiDir_Down, 1.0f / 3.0f, nullptr, &id);
        ImGuiID left = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 1.0f / 4.0f, nullptr, &id);
        ImGui::DockBuilderDockWindow(ICON_FA_BOOKMARK   "Project", down);
        ImGui::DockBuilderDockWindow(ICON_FA_DESKTOP    "Log", down);
        ImGui::DockBuilderDockWindow(ICON_FA_LAPTOP     "Lua Console", down);
        ImGui::DockBuilderDockWindow(ICON_FA_TABLET     "QuickJS Console", down);
        ImGui::DockBuilderDockWindow(ICON_FA_BAR_CHART  "Profiler", down);
        ImGui::DockBuilderDockWindow(ICON_FA_LIST       "Hierarchy", left);
        ImGui::DockBuilderDockWindow(ICON_FA_INFO_CIRCLE"Inspector", right);
        ImGui::DockBuilderDockWindow(ICON_FA_GLOBE      "Scene", id);
        ImGui::DockBuilderDockWindow(ICON_FA_GAMEPAD    "Game", id);
        ImGui::DockBuilderFinish(id);
    }

    updated |= About::Update(updateData, showAbout);
    updated |= Document::Update(updateData);
    updated |= Setup::Update(updateData, showSetup);
    updated |= Project::Update(updateData, showProject);
    updated |= Log::Update(updateData, showLog);
    updated |= LuaConsole::Update(updateData, showLuaConsole);
    updated |= QuickJSConsole::Update(updateData, showQuickJSConsole);
    updated |= Profiler::Update(updateData, showLuaConsole);
    updated |= Hierarchy::Update(updateData, showHierarchy, Scene::sceneRoot);
    updated |= Inspector::Update(updateData, showInspector, Scene::mainCamera);
    updated |= Scene::Update(updateData, showScene);
    updated |= Game::Update(updateData, showGame);
    updated |= ShaderAssembler::Update(updateData, showShaderAssembler);
    updated |= ShaderDisassembler::Update(updateData, showShaderDisassembler);

    static bool dockPostInitialized = false;
    if (dockPostInitialized == false)
    {
        dockPostInitialized = true;

        ImGui::SetWindowFocus(ICON_FA_BOOKMARK  "Project");
        ImGui::SetWindowFocus(ICON_FA_GLOBE     "Scene");
    }

    Runtime::Update();

    return updated;
}
//------------------------------------------------------------------------------
moduleAPI void Render(const RenderData& renderData)
{
}
//---------------------------------------------------------------------------
