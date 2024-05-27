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
    QuickJS::Initialize();

    xxLog("Runtime", Runtime::Version);
    xxLog("Runtime", "%s for %s", Runtime::Compiler, Runtime::Target);

    xxLog("Runtime", "FreeType " xxStringify(FREETYPE_MAJOR) "." xxStringify(FREETYPE_MINOR) "." xxStringify(FREETYPE_PATCH));
    xxLog("Runtime", Lua::Version);
    xxLog("Runtime", QuickJS::Version);
}
//------------------------------------------------------------------------------
void Runtime::Shutdown(bool suspend)
{
    if (initialized == false)
        return;
    initialized = false;

    VertexAttribute::Shutdown();
    Texture::Shutdown();
    Sampler::Shutdown();
    Shader::Shutdown();
    RenderPass::Shutdown();
    Pipeline::Shutdown();
    Modifier::Shutdown();
    Binding::Shutdown();

    if (suspend == false)
    {
        QuickJS::Shutdown();
        Lua::Shutdown();
    }

    xxLog("Runtime", "Shutdown");
}
//==============================================================================
//  Information
//==============================================================================
#if defined(__clang_version__) || defined(__GNUC__)
char const Runtime::Compiler[] = __VERSION__;
#elif defined(_MSC_FULL_VER)
char const Runtime::Compiler[] =
{
    'M', 'i', 'c', 'r', 'o', 's', 'o', 'f', 't', ' ', 'V', 'i', 's', 'u', 'a', 'l', ' ', 'C', '+', '+', ' ',
    '0' + _MSC_FULL_VER / 100000000 % 10,
    '0' + _MSC_FULL_VER / 10000000 % 10,
    '.',
    '0' + _MSC_FULL_VER / 1000000 % 10,
    '0' + _MSC_FULL_VER / 100000 % 10,
    '.',
    '0' + _MSC_FULL_VER / 10000 % 10,
    '0' + _MSC_FULL_VER / 1000 % 10,
    '0' + _MSC_FULL_VER / 100 % 10,
    '0' + _MSC_FULL_VER / 10 % 10,
    '0' + _MSC_FULL_VER / 1 % 10,
    0,
};
#endif
//------------------------------------------------------------------------------
#if __ARM_ARCH_9_4__ || __ARM_ARCH_9_4A__
char const Runtime::Target[] = "Armv9.4";
#elif __ARM_ARCH_9_3__ || __ARM_ARCH_9_3A__
char const Runtime::Target[] = "Armv9.3";
#elif __ARM_ARCH_9_2__ || __ARM_ARCH_9_2A__
char const Runtime::Target[] = "Armv9.3";
#elif __ARM_ARCH_9_1__ || __ARM_ARCH_9_1A__
char const Runtime::Target[] = "Armv9.1";
#elif __ARM_ARCH_9__ || __ARM_ARCH_9A__ || __ARM_ARCH == 9
char const Runtime::Target[] = "Armv9.0";
#elif __ARM_ARCH_8_9__ || __ARM_ARCH_8_9A__
char const Runtime::Target[] = "Armv8.9";
#elif __ARM_ARCH_8_8__ || __ARM_ARCH_8_8A__
char const Runtime::Target[] = "Armv8.8";
#elif __ARM_ARCH_8_7__ || __ARM_ARCH_8_7A__
char const Runtime::Target[] = "Armv8.7";
#elif __ARM_ARCH_8_6__ || __ARM_ARCH_8_6A__
char const Runtime::Target[] = "Armv8.6";
#elif __ARM_ARCH_8_5__ || __ARM_ARCH_8_5A__
char const Runtime::Target[] = "Armv8.5";
#elif __ARM_ARCH_8_4__ || __ARM_ARCH_8_4A__
char const Runtime::Target[] = "Armv8.4";
#elif __ARM_ARCH_8_3__ || __ARM_ARCH_8_3A__
char const Runtime::Target[] = "Armv8.3";
#elif __ARM_ARCH_8_2__ || __ARM_ARCH_8_2A__
char const Runtime::Target[] = "Armv8.3";
#elif __ARM_ARCH_8_1__ || __ARM_ARCH_8_1A__
char const Runtime::Target[] = "Armv8.1";
#elif __ARM_ARCH_8__ || __ARM_ARCH_8A__ || __ARM_ARCH == 8
char const Runtime::Target[] = "Armv8.0";
#endif
//------------------------------------------------------------------------------
#if defined(__clang_version__) && defined(__x86_64__)
#if __AVX512VL__
char const Runtime::Target[] = "x86-64-v4";
#elif __AVX2__
char const Runtime::Target[] = "x86-64-v3";
#elif __SSE4_2__
char const Runtime::Target[] = "x86-64-v2";
#elif __SSE2__
char const Runtime::Target[] = "x86-64";
#endif
#elif defined(_MSC_VER)
char const Runtime::Target[] = ""
#if __SSE__
"SSE"
#if __SSE2__
"/SSE2";
#if __SSE3__
"/SSE3"
#if __SSSE3__
"/SSSE3"
#if __SSE4_1__
"/SSE4.1"
#if __SSE4_2__
"/SSE4.2"
#if __AVX__
"/AVX"
#if __AVX2__
"/AVX2"
#if __AVX512VL__
"/AVX512"
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
//------------------------------------------------------------------------------
char const Runtime::Version[] =
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
