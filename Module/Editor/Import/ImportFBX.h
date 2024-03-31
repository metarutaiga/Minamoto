//==============================================================================
// Minamoto : ImportFBX Header
//
// Copyright (c) 2023-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#include "Import.h"

class ImportFBX : public Import
{
public:
    static xxNodePtr Create(char const* fbx);
};
