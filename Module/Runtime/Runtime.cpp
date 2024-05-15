//==============================================================================
// Minamoto : Runtime Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <freetype/freetype.h>
#include <lua/lua.h>
#include "Modifier/Modifier.h"
#include "Graphic/Binding.h"
#include "Graphic/Pipeline.h"
#include "Graphic/RenderPass.h"
#include "Graphic/Sampler.h"
#include "Graphic/Shader.h"
#include "Graphic/Texture.h"
#include "Graphic/VertexAttribute.h"
#include "Script/Lua.h"
#include "Runtime.h"

static char const version[] =
{
    'M', 'i', 'n', 'a', 'm', 'o', 't', 'o', ' ',
    __DATE__[7],
    __DATE__[8],
    __DATE__[9],
    __DATE__[10],
    '.',
    (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n') ? '1' :
    (__DATE__[0] == 'F' && __DATE__[1] == 'e' && __DATE__[2] == 'b') ? '2' :
    (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r') ? '3' :
    (__DATE__[0] == 'A' && __DATE__[1] == 'p' && __DATE__[2] == 'r') ? '4' :
    (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y') ? '5' :
    (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n') ? '6' :
    (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l') ? '7' :
    (__DATE__[0] == 'A' && __DATE__[1] == 'u' && __DATE__[2] == 'g') ? '8' :
    (__DATE__[0] == 'S') ? '9' :
    (__DATE__[0] == 'O') ? '1' :
    (__DATE__[0] == 'N') ? '1' :
    (__DATE__[0] == 'D') ? '1' : '0',
    (__DATE__[0] == 'O') ? '0' :
    (__DATE__[0] == 'N') ? '1' :
    (__DATE__[0] == 'D') ? '2' : '.',
    (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? '.'                                           : __DATE__[4] > '0' ? __DATE__[4] : __DATE__[5],
    (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? __DATE__[4] > '0' ? __DATE__[4] : __DATE__[5] : __DATE__[4] > '0' ? __DATE__[5] : 0,
    (__DATE__[0] == 'O' || __DATE__[0] == 'N' || __DATE__[0] == 'D') ? __DATE__[4] > '0' ? __DATE__[5] : 0           : 0,
    0
};

//==============================================================================
static bool initialized = false;
//------------------------------------------------------------------------------
void Runtime::Initialize()
{
    if (initialized)
        return;
    initialized = true;

    Binding::Initialize();
    Modifier::Initialize();
    Pipeline::Initialize();
    RenderPass::Initialize();
    Shader::Initialize();
    Sampler::Initialize();
    Texture::Initialize();
    VertexAttribute::Initialize();
    Lua::Initialize();

    xxLog("Runtime", Version());
#if defined(__clang_version__)
    xxLog("Runtime", "clang " __clang_version__);
#elif defined(__GNUC__)
    xxLog("Runtime", "gcc " xxStringify(__GNUC__) "." xxStringify(__GNUC_MINOR__) "." xxStringify(__GNUC_PATCHLEVEL__));
#elif defined(_MSC_FULL_VER)
    xxLog("Runtime", "Visual C++ %d.%d.%d", _MSC_FULL_VER / 10000000 % 100, _MSC_FULL_VER / 100000 % 100, _MSC_FULL_VER % 100000);
#endif
    xxLog("Runtime", "FreeType " xxStringify(FREETYPE_MAJOR) "." xxStringify(FREETYPE_MINOR) "." xxStringify(FREETYPE_PATCH));
    xxLog("Runtime", "Lua " LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE);
}
//------------------------------------------------------------------------------
void Runtime::Shutdown()
{
    if (initialized == false)
        return;
    initialized = false;

    Lua::Shutdown();
    VertexAttribute::Shutdown();
    Texture::Shutdown();
    Sampler::Shutdown();
    Shader::Shutdown();
    RenderPass::Shutdown();
    Pipeline::Shutdown();
    Modifier::Shutdown();
    Binding::Shutdown();

    xxLog("Runtime", "Shutdown");
}
//------------------------------------------------------------------------------
char const* Runtime::Version()
{
    return version;
}
//==============================================================================
