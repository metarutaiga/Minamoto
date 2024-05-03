/*
** $Id: luauser.h $
** See Copyright Notice in lua.h
*/

#ifndef luauser_h
#define luauser_h

#undef LUA_API
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define LUA_API __declspec(dllexport)
#else
#define LUA_API __attribute__((visibility("default")))
#endif

#define lua_writestring lua_writestring
#define lua_writeline lua_writeline
LUA_API void lua_writestring(const char* string, size_t length);
LUA_API void lua_writeline(void);

#endif
