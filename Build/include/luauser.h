/*
** $Id: luauser.h $
** See Copyright Notice in lua.h
*/

#ifndef luauser_h
#define luauser_h

#undef LUA_API
#undef LUALIB_API
#undef LUAMOD_API
#if defined(_WIN32)
#define LUA_API     __declspec(dllexport)
#define LUALIB_API
#define LUAMOD_API
#define WIN32_LEAN_AND_MEAN
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

#define lua_writestring lua_writestring
#define lua_writeline lua_writeline
#define lua_writestringerror lua_writestringerror
LUA_API void lua_writestring(const char* string, size_t length);
LUA_API void lua_writeline(void);
LUA_API void lua_writestringerror(const char* string, const char* parameter);

#endif
