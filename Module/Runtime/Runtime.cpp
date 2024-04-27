//==============================================================================
// Minamoto : Runtime Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Modifier/Modifier.h"
#include "Graphic/Shader.h"
#include "Runtime.h"

//==============================================================================
void Runtime::Initialize()
{
    Modifier::Initialize();
    Shader::Initialize();
}
//------------------------------------------------------------------------------
void Runtime::Shutdown()
{
    Shader::Shutdown();
    Modifier::Shutdown();
}
//==============================================================================
