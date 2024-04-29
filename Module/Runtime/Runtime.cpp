//==============================================================================
// Minamoto : Runtime Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include <xxSystem.h>
#include <freetype/freetype.h>
#include "Modifier/Modifier.h"
#include "Graphic/Pipeline.h"
#include "Graphic/RenderPass.h"
#include "Graphic/Shader.h"
#include "Graphic/VertexAttribute.h"
#include "Runtime.h"

//==============================================================================
static bool initialized = false;
//------------------------------------------------------------------------------
void Runtime::Initialize()
{
    if (initialized)
        return;
    initialized = true;

    Modifier::Initialize();
    Pipeline::Initialize();
    RenderPass::Initialize();
    Shader::Initialize();
    VertexAttribute::Initialize();

#if defined(__clang_version__)
    xxLog("Runtime", "clang " __clang_version__);
#elif defined(__GNUC__)
    xxLog("Runtime", "gcc " xxStringify(__GNUC__) "." xxStringify(__GNUC_MINOR__) "." xxStringify(__GNUC_PATCHLEVEL__));
#elif defined(_MSC_FULL_VER)
    xxLog("Runtime", "Visual C++ %d.%d.%d", _MSC_FULL_VER / 10000000 % 100, _MSC_FULL_VER / 100000 % 100, _MSC_FULL_VER % 100000);
#endif
    xxLog("Runtime", "FreeType " xxStringify(FREETYPE_MAJOR) "." xxStringify(FREETYPE_MINOR) "." xxStringify(FREETYPE_PATCH));
}
//------------------------------------------------------------------------------
void Runtime::Shutdown()
{
    if (initialized == false)
        return;
    initialized = false;

    VertexAttribute::Shutdown();
    Shader::Shutdown();
    RenderPass::Shutdown();
    Pipeline::Shutdown();
    Modifier::Shutdown();
}
//==============================================================================
