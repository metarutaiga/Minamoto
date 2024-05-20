//==============================================================================
// Minamoto : LuaConsole Source
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Editor.h"
#include <queue>
#include "Script/Lua.h"
#include "Component/Console.h"
#include "LuaConsole.h"

extern "C"
{
#define LUA_USER_H "../../Build/include/luauser.h"
#include <lua/lua.h>
}

static Console console;
//------------------------------------------------------------------------------
extern "C"
{
#define lua_initreadline(L)
#define lua_readline lua_readline
#define lua_saveline(L, line) lua_saveline(L, line)
#define lua_freeline(L, b)
static int lua_readline(lua_State* L, char* buffer, char const* prompt);
static int lua_saveline(lua_State* L, char const* line);
#define main lua_main
#include <lua/lua.c>
#undef main
}
//------------------------------------------------------------------------------
static int lua_readline(lua_State* L, char* buffer, char const* prompt)
{
    auto& line = Lua::lines.back();
    if (line != prompt)
    {
        for (char c; (c = *prompt); ++prompt)
        {
            if (c == '\b' || c == 0x7F)
            {
                if (line.empty() == false)
                    line.pop_back();
                continue;
            }
            line.append(1, c);
        }
    }
    auto& input = console.input;
    auto& inputPos = console.inputPos;
    if (input.empty() || input[0] == 0)
        return 0;
    lua_writestring(input.c_str(), input.size());
    lua_writeline();
    strncpy(buffer, input.c_str(), LUA_MAXINPUT);
    size_t erase = std::min<size_t>(input.size(), LUA_MAXINPUT);
    input.erase(input.begin(), input.begin() + erase);
    inputPos -= inputPos;
    return 1;
}
//------------------------------------------------------------------------------
static int lua_saveline(lua_State* L, char const* line)
{
    console.AddHistory(line);
    return 0;
}
//------------------------------------------------------------------------------
void LuaConsole::Initialize()
{
    Lua::StandardLibrary();
    print_version();
}
//------------------------------------------------------------------------------
void LuaConsole::Shutdown()
{
}
//------------------------------------------------------------------------------
bool LuaConsole::Update(const UpdateData& updateData, bool& show)
{
    if (show == false)
        return false;

    bool update = false;
    if (ImGui::Begin(ICON_FA_LAPTOP "Lua Console", &show))
    {
        if (ImGui::IsWindowFocused())
        {
            update |= console.UpdateInput(updateData);
        }

        bool appearing = ImGui::IsWindowAppearing();
        if (appearing || ImGui::IsKeyReleased(ImGuiKey_Enter))
        {
            auto& input = console.input;

            char c = 0;
            if (appearing && input.empty() == false)
                std::swap(input[0], c);

            auto L = Lua::L;
            lua_initreadline(L);
            for (size_t i = 0; i < 2; ++i)
            {
                int status;
                if ((status = loadline(L)) != -1)
                {
                    if (status == LUA_OK)
                        status = docall(L, 0, LUA_MULTRET);
                    if (status == LUA_OK)
                        l_print(L);
                    else
                        report(L, status);
                }
            }
            lua_settop(L, 0);  /* clear stack */
            update = true;

            if (appearing && input.empty() == false)
                std::swap(input[0], c);
        }

        auto const& lines = Lua::lines;
        update |= console.UpdateConsole(updateData, lines);
    }
    ImGui::End();

    return update;
}
//------------------------------------------------------------------------------
