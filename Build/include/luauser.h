/*
** $Id: luauser.h $
** See Copyright Notice in lua.h
*/

#ifndef luauser_h
#define luauser_h

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wconditional-uninitialized"
#pragma clang diagnostic ignored "-Wunused-function"
#endif

#undef LUA_API
#undef LUALIB_API
#undef LUAMOD_API
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define LUA_API     __declspec(dllexport)
#define LUALIB_API
#define LUAMOD_API
#define LUA_DL_DLL
#define LUA_USE_C89
#else
#define LUA_API     __attribute__((visibility("default")))
#define LUALIB_API
#define LUAMOD_API
#define LUA_USE_DLOPEN
#define LUA_USE_POSIX
#endif

#undef LUA_PATH_DEFAULT
#undef LUA_CPATH_DEFAULT
#define LUA_PATH_DEFAULT ""
#define LUA_CPATH_DEFAULT ""

#define lua_initreadline lua_pinitreadline
#define lua_readline lua_preadline
#define lua_saveline lua_psaveline
#define lua_freeline lua_pfreeline
LUA_API extern void (*lua_pinitreadline)(lua_State* L);
LUA_API extern int  (*lua_preadline)(lua_State* L, char* buffer, char const* prompt);
LUA_API extern void (*lua_psaveline)(lua_State* L, char const* line);
LUA_API extern void (*lua_pfreeline)(lua_State* L, char* buffer);

#define lua_writeline lua_pwriteline
#define lua_writestring lua_pwritestring
#define lua_writestringerror lua_pwritestringerror
LUA_API extern void (*lua_pwriteline)(void);
LUA_API extern void (*lua_pwritestring)(const char* string, size_t length);
LUA_API extern void (*lua_pwritestringerror)(const char* string, const char* parameter);

#endif
