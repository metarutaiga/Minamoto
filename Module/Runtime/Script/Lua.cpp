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
}

lua_State* Lua::L;
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
    }
}
//------------------------------------------------------------------------------
void Lua::Shutdown()
{
    lua_close(L);

    L = nullptr;
}
//------------------------------------------------------------------------------
char const* Lua::Version()
{
    return "Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE;
}
//==============================================================================
