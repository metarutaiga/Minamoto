//==============================================================================
// Minamoto : Runtime Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#ifndef RuntimeAPI
#   if defined(_MSC_VER) && defined(RUNTIME_BUILD_LIBRARY)
#       define RuntimeAPI           __declspec(dllexport)
#   elif defined(_MSC_VER)
#       define RuntimeAPI           __declspec(dllimport)
#   else
#       define RuntimeAPI           __attribute__((visibility("default")))
#   endif
#endif

struct RuntimeAPI Runtime
{
    static void Initialize();
    static void Shutdown();
};
