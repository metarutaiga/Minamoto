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

#ifndef LUAEX_API
#if defined(_WIN32)
#define LUAEX_API   __declspec(dllexport)
#else
#define LUAEX_API   __attribute__((visibility("default")))
#endif
#elif defined(_WIN32)
#undef LUAEX_API
#define LUAEX_API   __declspec(dllimport)
#endif

#define lua_initreadline lua_pinitreadline
#define lua_readline lua_preadline
#define lua_saveline lua_psaveline
#define lua_freeline lua_pfreeline
LUAEX_API extern void (*lua_pinitreadline)(lua_State* L);
LUAEX_API extern int  (*lua_preadline)(lua_State* L, char* buffer, char const* prompt);
LUAEX_API extern void (*lua_psaveline)(lua_State* L, char const* line);
LUAEX_API extern void (*lua_pfreeline)(lua_State* L, char* buffer);

#define lua_writeline lua_pwriteline
#define lua_writestring lua_pwritestring
#define lua_writestringerror lua_pwritestringerror
LUAEX_API extern void (*lua_pwriteline)(void);
LUAEX_API extern void (*lua_pwritestring)(const char* string, size_t length);
LUAEX_API extern void (*lua_pwritestringerror)(const char* string, const char* parameter);

#endif
