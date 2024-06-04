//==============================================================================
// Minamoto : Font Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#if HAVE_MINIGUI

#include "Runtime.h"

namespace MiniGUI
{
struct RuntimeAPI Font
{
    struct CharGlyph
    {
        struct int16x2_t
        {
            int16_t x;
            int16_t y;
        };

        int16x2_t   rectLT = {};
        int16x2_t   rectRB = {};
        xxVector2   uvLT = {};
        xxVector2   uvRB = {};
        float       advance = 0.0f;
    };

public:
    static void             Initialize();
    static void             Shutdown(bool suspend = false);
    static xxVector2        Extent(std::string_view text, float scale);
    static CharGlyph const* Glyph(char32_t codepoint);
    static xxMaterialPtr    Material();
    static xxMeshPtr        Mesh(xxMeshPtr const& mesh, std::string_view text, xxMatrix3x4 const color, xxVector2 const& scale, float shadow);
    static xxMeshPtr        MeshColor(xxMeshPtr const& mesh, xxMatrix3x4 const color);
    static xxMeshPtr        MeshScale(xxMeshPtr const& mesh, xxVector2 const& scale);
    static xxMeshPtr        MeshShadow(xxMeshPtr const& mesh, float shadow);
    static uint32_t         ToCodePoint(std::string_view& text);
    static std::string      ToUTF8(std::u32string const& text);
    static std::u32string   ToUTF32(std::string_view text);
};
}   // namespace MiniGUI
#endif
