//==============================================================================
// Minamoto : Lua Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <queue>
#include <string>
#include "Lua.h"

extern "C"
{
#define LUA_USER_H "../../Build/include/luauser.h"
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
}

lua_State* Lua::L;
std::deque<std::string> Lua::lines;
//==============================================================================
void Lua::Initialize()
{
    lines.push_back(std::string());

    L = luaL_newstate();  /* create state */
}
//------------------------------------------------------------------------------
void Lua::StandardLibrary()
{
    lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
    luaL_openlibs(L);  /* open standard libraries */
    lua_gc(L, LUA_GCRESTART);  /* start GC... */
    lua_gc(L, LUA_GCGEN, 0, 0);  /* ...in generational mode */
}
//------------------------------------------------------------------------------
void Lua::Shutdown()
{
    lua_close(L);

    lines = std::deque<std::string>();
}
//------------------------------------------------------------------------------
void lua_writestring(char const* string, size_t length)
{
    for (size_t i = 0; i < length; ++i)
    {
        auto& line = Lua::lines.back();
        char c = string[i];
        switch (c)
        {
        case 0:
            return;
        case '\b':
        case 0x7F:
            if (line.empty() == false)
                line.pop_back();
            continue;
        case '\n':
            Lua::lines.push_back(std::string());
            continue;
        case '\r':
            if (i + 1 < length && string[i + 1] != '\n')
                line.clear();
            continue;
        case '\t':
            line.resize((line.size() + 3) & ~3, ' ');
            continue;
        default:
            line.append(1, c);
            break;
        }
    }
}
//------------------------------------------------------------------------------
void lua_writeline()
{
    Lua::lines.push_back(std::string());
}
//------------------------------------------------------------------------------
void lua_writestringerror(char const* string, char const* parameter)
{
    size_t count = snprintf(nullptr, 0, string, parameter);
    char* temp = xxAlloc(char, count + 1);
    snprintf(temp, count + 1, string, parameter);
    lua_writestring(temp, count);
    xxFree(temp);
}
//==============================================================================
