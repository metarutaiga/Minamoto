#pragma once

struct Grid
{
    static xxNodePtr Create(xxVector3 const& translate, xxVector2 const& size);
    static xxImagePtr CreateTexture();
};
