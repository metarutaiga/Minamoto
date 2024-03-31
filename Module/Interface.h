//==============================================================================
// Minamoto : Module Interface Header
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <xxGraphic/xxSystem.h>
#include <imgui/imgui.h>
#include <initializer_list>

#if defined(_MSC_VER) && defined(MODULE_BUILD_LIBRARY)
#   define moduleAPI extern "C" __declspec(dllexport)
#elif defined(_MSC_VER)
#   define moduleAPI extern "C" __declspec(dllimport)
#else
#   define moduleAPI extern "C" __attribute__((visibility("default")))
#endif

struct CreateData
{
    uint64_t    device;
    const char* baseFolder;
};

struct ShutdownData
{

};

struct MessageData
{
    const char* const*  data;
    size_t              length;
};

struct UpdateData
{
    uint64_t    instance;
    uint64_t    device;
    uint64_t    renderPass;
    int         width;
    int         height;
    float       scale;
    float       time;
    float       elapsed;
    void      (*message)(std::initializer_list<const char*> list);
};

struct RenderData
{
    uint64_t    instance;
    uint64_t    device;
    uint64_t    renderPass;
    uint64_t    commandBuffer;
    uint64_t    commandEncoder;
    uint64_t    commandFramebuffer;
    int         width;
    int         height;
    float       scale;
};

typedef const char* (*PFN_MODULE_CREATE)(const CreateData&);
typedef void (*PFN_MODULE_SHUTDOWN)(const ShutdownData&);
typedef void (*PFN_MODULE_MESSAGE)(const MessageData&);
typedef bool (*PFN_MODULE_UPDATE)(const UpdateData&);
typedef void (*PFN_MODULE_RENDER)(const RenderData&);

namespace ImGui
{
inline void DumpBuildInformation()
{
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED)
    ImGui::Text("macOS : %d.%d.%d", __MAC_OS_X_VERSION_MAX_ALLOWED / 10000, __MAC_OS_X_VERSION_MAX_ALLOWED / 100 % 100, __MAC_OS_X_VERSION_MAX_ALLOWED % 100);
#endif
#if defined(__clang_version__)
    ImGui::Text("clang : %s", __clang_version__);
#endif
#if defined(_LIBCPP_VERSION)
    ImGui::Text("libc++ : %d.%d.%d", _LIBCPP_VERSION / 10000, _LIBCPP_VERSION / 100 % 100, _LIBCPP_VERSION % 100);
#endif
#if defined(__GNUC__)
    ImGui::Text("gcc : %d.%d.%d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
#if defined(__GLIBC__)
    ImGui::Text("glibc : %d.%d", __GLIBC__, __GLIBC_MINOR__);
#endif
#if defined(__GLIBCXX__)
    ImGui::Text("libstdc++ : %d.%d.%d", __GLIBCXX__ / 10000, __GLIBCXX__ / 100 % 100, __GLIBCXX__ % 100);
#endif
#if defined(_MSC_FULL_VER)
    ImGui::Text("msvc : %d.%d.%d", _MSC_FULL_VER / 10000000 % 100, _MSC_FULL_VER / 100000 % 100, _MSC_FULL_VER % 100000);
#endif
#if defined(_CPPLIB_VER)
    ImGui::Text("Dinkumware : %d.%d", _CPPLIB_VER / 100, _CPPLIB_VER % 100);
#endif
#if defined(_MSVC_STL_VERSION)
    ImGui::Text("msstl : %d.%d.%ld", _MSVC_STL_VERSION / 10, _MSVC_STL_VERSION % 10, _MSVC_STL_UPDATE);
#endif
}
}
