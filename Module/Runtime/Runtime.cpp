//==============================================================================
// Minamoto : Runtime Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <freetype/freetype.h>
#include "Modifier/Modifier.h"
#include "Graphic/Binding.h"
#include "Graphic/Pipeline.h"
#include "Graphic/RenderPass.h"
#include "Graphic/Sampler.h"
#include "Graphic/Shader.h"
#include "Graphic/Texture.h"
#include "Graphic/VertexAttribute.h"
#include "Script/Lua.h"
#include "Script/QuickJS.h"
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
    (__DATE__[0] == 'S' && __DATE__[1] == 'e' && __DATE__[2] == 'p') ? '9' :
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
#if defined(__clang_version__)
static void ClangTarget()
{
#if __ARM_ARCH_9_4__ || __ARM_ARCH_9_4A__
    xxLog("Runtime", "clang target is Armv9.4");
#elif __ARM_ARCH_9_3__ || __ARM_ARCH_9_3A__
    xxLog("Runtime", "clang target is Armv9.3");
#elif __ARM_ARCH_9_2__ || __ARM_ARCH_9_2A__
    xxLog("Runtime", "clang target is Armv9.3");
#elif __ARM_ARCH_9_1__ || __ARM_ARCH_9_1A__
    xxLog("Runtime", "clang target is Armv9.1");
#elif __ARM_ARCH_9__ || __ARM_ARCH_9A__ || __ARM_ARCH == 9
    xxLog("Runtime", "clang target is Armv9.0");
#elif __ARM_ARCH_8_9__ || __ARM_ARCH_8_9A__
    xxLog("Runtime", "clang target is Armv8.9");
#elif __ARM_ARCH_8_8__ || __ARM_ARCH_8_8A__
    xxLog("Runtime", "clang target is Armv8.8");
#elif __ARM_ARCH_8_7__ || __ARM_ARCH_8_7A__
    xxLog("Runtime", "clang target is Armv8.7");
#elif __ARM_ARCH_8_6__ || __ARM_ARCH_8_6A__
    xxLog("Runtime", "clang target is Armv8.6");
#elif __ARM_ARCH_8_5__ || __ARM_ARCH_8_5A__
    xxLog("Runtime", "clang target is Armv8.5");
#elif __ARM_ARCH_8_4__ || __ARM_ARCH_8_4A__
    xxLog("Runtime", "clang target is Armv8.4");
#elif __ARM_ARCH_8_3__ || __ARM_ARCH_8_3A__
    xxLog("Runtime", "clang target is Armv8.3");
#elif __ARM_ARCH_8_2__ || __ARM_ARCH_8_2A__
    xxLog("Runtime", "clang target is Armv8.3");
#elif __ARM_ARCH_8_1__ || __ARM_ARCH_8_1A__
    xxLog("Runtime", "clang target is Armv8.1");
#elif __ARM_ARCH_8__ || __ARM_ARCH_8A__ || __ARM_ARCH == 8
    xxLog("Runtime", "clang target is Armv8.0");
#endif

#if __AVX512VL__
    xxLog("Runtime", "clang target is x86-64-v4");
#elif __AVX2__
    xxLog("Runtime", "clang target is x86-64-v3");
#elif __SSE4_2__
    xxLog("Runtime", "clang target is x86-64-v2");
#elif __SSE2__
    xxLog("Runtime", "clang target is x86-64");
#endif
}
#elif defined(_MSC_FULL_VER)
static void MSVCTarget()
{
#if __AVX512VL__
    xxLog("Runtime", "Visual C++ target is AVX512");
#elif __AVX2__
    xxLog("Runtime", "Visual C++ target is AVX2");
#elif __AVX__
    xxLog("Runtime", "Visual C++ target is AVX");
#elif __SSE4_2__
    xxLog("Runtime", "Visual C++ target is SSE4.2");
#elif __SSE4_1__
    xxLog("Runtime", "Visual C++ target is SSE4.1");
#elif __SSSE3__
    xxLog("Runtime", "Visual C++ target is SSSE3");
#elif __SSE3__
    xxLog("Runtime", "Visual C++ target is SSE3");
#elif __SSE2__
    xxLog("Runtime", "Visual C++ target is SSE2");
#endif
}
#endif
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
    QuickJS::Initialize();

    xxLog("Runtime", Version());
#if defined(__clang_version__)
    xxLog("Runtime", "clang " __clang_version__);
    ClangTarget();
#elif defined(__GNUC__)
    xxLog("Runtime", "gcc " xxStringify(__GNUC__) "." xxStringify(__GNUC_MINOR__) "." xxStringify(__GNUC_PATCHLEVEL__));
#elif defined(_MSC_FULL_VER)
    xxLog("Runtime", "Visual C++ %d.%d.%d", _MSC_FULL_VER / 10000000 % 100, _MSC_FULL_VER / 100000 % 100, _MSC_FULL_VER % 100000);
    MSVCTarget();
#endif

    xxLog("Runtime", "FreeType " xxStringify(FREETYPE_MAJOR) "." xxStringify(FREETYPE_MINOR) "." xxStringify(FREETYPE_PATCH));
    xxLog("Runtime", Lua::Version());
    xxLog("Runtime", QuickJS::Version());
}
//------------------------------------------------------------------------------
void Runtime::Shutdown()
{
    if (initialized == false)
        return;
    initialized = false;

    QuickJS::Shutdown();
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
