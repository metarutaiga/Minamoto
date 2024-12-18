//==============================================================================
// Minamoto : Runtime Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include <xxGraphic/xxGraphic.h>
#include <xxGraphicPlus/xxUtility.h>

#ifndef RuntimeAPI
#   if defined(_MSC_VER) && defined(RUNTIME_BUILD_LIBRARY)
#       define RuntimeAPI           __declspec(dllexport)
#   elif defined(_MSC_VER)
#       define RuntimeAPI           __declspec(dllimport)
#   else
#       define RuntimeAPI           __attribute__((visibility("default")))
#   endif
#endif

#if defined(_MSC_VER)
#   pragma warning(disable:4200)
#   pragma warning(disable:4251)
#endif

struct RuntimeAPI Runtime
{
    static void Initialize();
    static void Update();
    static void Shutdown(bool suspend = false);
    static char const Compiler[];
    static char const Target[];
    static char const Version[];
};

#define HAVE_MINIGUI 1
#if HAVE_MINIGUI
namespace MiniGUI
{
typedef std::shared_ptr<class Window> WindowPtr;
}
#endif

#define xxFourCC
#define xxEightCC
constexpr uint32_t operator ""_cc(char const* text, size_t length)
{
    uint32_t value = 0;
    for (size_t i = 0; i < length; ++i)
        value += uint32_t(uint8_t(text[i])) << (i * 8);
    return value;
};
constexpr uint64_t operator ""_CC(char const* text, size_t length)
{
    uint64_t value = 0;
    for (size_t i = 0; i < length; ++i)
        value += uint64_t(uint8_t(text[i])) << (i * 8);
    return value;
};
