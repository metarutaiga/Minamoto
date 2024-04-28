//==============================================================================
// Minamoto : Runtime Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Modifier/Modifier.h"
#include "Graphic/Pipeline.h"
#include "Graphic/RenderPass.h"
#include "Graphic/Shader.h"
#include "Graphic/VertexAttribute.h"
#include "Runtime.h"

//==============================================================================
void Runtime::Initialize()
{
    Modifier::Initialize();
    Pipeline::Initialize();
    RenderPass::Initialize();
    Shader::Initialize();
    VertexAttribute::Initialize();
}
//------------------------------------------------------------------------------
void Runtime::Shutdown()
{
    VertexAttribute::Shutdown();
    Shader::Shutdown();
    RenderPass::Shutdown();
    Pipeline::Shutdown();
    Modifier::Shutdown();
}
//==============================================================================
