//==============================================================================
// Minamoto : Grid Header
//
// Copyright (c) 2019-2023 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

struct Grid
{
    static xxNodePtr Create(xxVector3 const& translate, xxVector2 const& size);
    static xxImagePtr CreateTexture();
};
