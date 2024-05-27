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
#define LUA_LIB
#define LUA_USER_H "../../Build/include/luauser.h"
#include <lua/lua.h>
#include <lua/lauxlib.c>
#include <lua/lbaselib.c>
#include <lua/lcorolib.c>
#include <lua/ldblib.c>
#include <lua/lmathlib.c>
#include <lua/lstrlib.c>
#include <lua/ltablib.c>
#include <lua/lutf8lib.c>
}

lua_State* Lua::L;
char const Lua::Version[] = "Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE;
//==============================================================================
void Lua::Initialize()
{
    if (L == nullptr)
    {
        L = lua_newstate([](void*, void* ptr, size_t osize, size_t nsize) -> void*
        {
            if (nsize == 0)
            {
                xxFree(ptr);
                return NULL;
            }
            return xxRealloc(ptr, char, nsize);
        }, nullptr);  /* create state */

        lua_pinitreadline = [](lua_State*){};
        lua_preadline = [](lua_State*, char*, char const*){ return 0; };
        lua_psaveline = [](lua_State*, char const*){};
        lua_pfreeline = [](lua_State*, char*){};
        lua_pwriteline = []{};
        lua_pwritestring = [](char const*, size_t){};
        lua_pwritestringerror = [](char const*, char const*){};

        static luaL_Reg const loadedlibs[] =
        {
            { LUA_GNAME, luaopen_base },
            { LUA_COLIBNAME, luaopen_coroutine },
            { LUA_TABLIBNAME, luaopen_table },
            { LUA_STRLIBNAME, luaopen_string },
            { LUA_MATHLIBNAME, luaopen_math },
            { LUA_UTF8LIBNAME, luaopen_utf8 },
            { LUA_DBLIBNAME, luaopen_debug },
            { NULL, NULL}
        };

        lua_gc(L, LUA_GCSTOP);  /* stop GC while building state */
        for (luaL_Reg const* lib = loadedlibs; lib->func; lib++)
        {
            luaL_requiref(L, lib->name, lib->func, 1);
            lua_pop(L, 1);  /* remove lib */
        }
        lua_gc(L, LUA_GCRESTART);  /* start GC... */
        lua_gc(L, LUA_GCGEN, 0, 0);  /* ...in generational mode */
    }
}
//------------------------------------------------------------------------------
void Lua::Shutdown()
{
    lua_close(L);

    L = nullptr;
}
//------------------------------------------------------------------------------
void Lua::RuntimeLibrary()
{
}
//------------------------------------------------------------------------------
void Lua::Eval(char const* buf, size_t len)
{
    std::pair<char const*, size_t> pair(buf, len);
    lua_load(L, [](lua_State *L, void* ud, size_t* size) -> char const*
    {
        auto& pair = *(std::pair<char const*, size_t>*)ud;
        if (pair.second == 0)
            return nullptr;
        (*size) = pair.second;
        pair.second = 0;
        return pair.first;
    }, &pair, "<EVAL>", nullptr);
}
//------------------------------------------------------------------------------
void Lua::Update()
{
    lua_pcall(L, 0, 0, 0);
}
//==============================================================================
//  Standard I/O
//==============================================================================
LUA_API void (*lua_pinitreadline)(lua_State* L);
LUA_API int  (*lua_preadline)(lua_State* L, char* buffer, char const* prompt);
LUA_API void (*lua_psaveline)(lua_State* L, char const* line);
LUA_API void (*lua_pfreeline)(lua_State* L, char* buffer);
LUA_API void (*lua_pwriteline)(void);
LUA_API void (*lua_pwritestring)(const char* string, size_t length);
LUA_API void (*lua_pwritestringerror)(const char* string, const char* parameter);
//==============================================================================
