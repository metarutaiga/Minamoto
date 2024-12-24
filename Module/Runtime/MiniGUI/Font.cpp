//==============================================================================
// Minamoto : Font Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <vector>
#include <freetype/freetype.h>
#include <xxGraphicPlus/xxMaterial.h>
#include <xxGraphicPlus/xxMesh.h>
#include <xxGraphicPlus/xxTexture.h>
#include "Font.h"

#if HAVE_MINIGUI
namespace MiniGUI
{
//==============================================================================
#define INT_TO_F26DOT6(x)   ((FT_Long)(x) * 64)
#define F26DOT6_TO_INT(x)   ((FT_Long)(x) / 64)
#define SIZE                28
#define SPACE               1024
#define GAP                 1
#define FAIL                -1
//------------------------------------------------------------------------------
static FT_Library library;
static FT_Face face;
static xxMaterialPtr material;
static xxTexturePtr texture;
static unsigned int textureMaxHeight;
static unsigned int textureStepX;
static unsigned int textureStepY;
static std::vector<Font::CharGlyph> codepoints;
//==============================================================================
void Font::Initialize()
{
    FT_Init_FreeType(&library);
    FT_New_Face(library, "/System/Library/Fonts/STHeiti Medium.ttc", 0, &face);
    if (face)
    {
        FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        FT_Size_RequestRec request =
        {
            .type = FT_SIZE_REQUEST_TYPE_REAL_DIM,
            .height = INT_TO_F26DOT6(SIZE),
        };
        FT_Request_Size(face, &request);
    }
    texture = xxTexture::Create2D("RGBA8888"_CC, SPACE, SPACE, 1);
    memset((*texture)(), 0, texture->Width * texture->Height * sizeof(uint32_t));
    material = xxMaterial::Create();
    material->AmbientColor = xxVector3::ONE;
    material->AlphaTest = true;
    material->AlphaTestReference = 0.25f;
    material->SetTexture(0, texture);
}
//------------------------------------------------------------------------------
void Font::Shutdown(bool suspend)
{
    if (suspend)
    {
        texture->Invalidate();
        return;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    material = nullptr;
    texture = nullptr;
    codepoints = std::vector<CharGlyph>();
}
//------------------------------------------------------------------------------
xxVector2 Font::Extent(std::string_view text, float scale)
{
    xxVector2 extent = xxVector2::ZERO;
    while (char32_t w = ToCodePoint(text))
    {
        CharGlyph const* glyph = Glyph(w);
        if (glyph == nullptr)
            continue;
        extent.x += glyph->advance * (scale / SIZE);
    }
    extent.y = SIZE * (scale / SIZE);
    return extent;
}
//------------------------------------------------------------------------------
Font::CharGlyph const* Font::Glyph(char32_t codepoint)
{
    if (codepoint > 0x10FFFF)
        return nullptr;
    if (codepoints.size() <= codepoint)
        codepoints.resize(codepoint + 1);
    CharGlyph& glyph = codepoints[codepoint];
    xxLocalBreak()
    {
        if (glyph.rectLT.x != 0.0f)
            break;
        if (FT_Load_Char(face, codepoint, FT_LOAD_DEFAULT) != FT_Err_Ok ||
            FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF) != FT_Err_Ok)
        {
            glyph.rectLT.x = FAIL;
            break;
        }
        FT_GlyphSlot const& slot = face->glyph;
        FT_Bitmap const& bitmap = slot->bitmap;
        if (textureStepX + bitmap.width >= SPACE)
        {
            textureStepX = 0;
            textureStepY += textureMaxHeight + GAP;
            textureMaxHeight = 0;
        }
        if (textureStepY + bitmap.rows >= SPACE)
        {
            glyph.rectLT.x = FAIL;
            break;
        }
        glyph.rectLT.x = slot->bitmap_left;
        glyph.rectLT.y = -slot->bitmap_top;
        glyph.rectRB.x = slot->bitmap_left + bitmap.width;
        glyph.rectRB.y = -slot->bitmap_top + bitmap.rows;
        glyph.uvLT.x = float(textureStepX) / SPACE;
        glyph.uvLT.y = float(textureStepY) / SPACE;
        glyph.uvRB.x = float(textureStepX + bitmap.width) / SPACE;
        glyph.uvRB.y = float(textureStepY + bitmap.rows) / SPACE;
        glyph.advance = F26DOT6_TO_INT(slot->advance.x + INT_TO_F26DOT6(GAP + GAP));
        switch (bitmap.pixel_mode)
        {
        case FT_PIXEL_MODE_GRAY:
        {
            uint8_t* input = (uint8_t*)bitmap.buffer;
            uint32_t* output = (uint32_t*)(*texture)(textureStepX, textureStepY);
            for (unsigned int y = 0; y < bitmap.rows; ++y)
            {
                uint8_t* gray = input;
                uint32_t* pixel = output;
                for (unsigned int x = 0; x < bitmap.width; ++x)
                {
                    (*pixel++) = ((*gray++) << 24) | 0x00FFFFFF;
                }
                input += bitmap.pitch;
                output += SPACE;
            }
            textureMaxHeight = std::max(textureMaxHeight, bitmap.rows);
            textureStepX += bitmap.width + GAP;
            texture->Dirty = true;
            break;
        }
        default:
            break;
        }
    }
    return glyph.rectLT.x != FAIL ? &glyph : nullptr;
}
//------------------------------------------------------------------------------
xxMaterialPtr Font::Material()
{
    return material;
}
//------------------------------------------------------------------------------
xxMeshPtr Font::Mesh(xxMeshPtr const& mesh, std::string_view text, xxMatrix3x4 const color, xxVector2 const& scale, float shadow)
{
    // Calculate text counts
    int textCount = 0;
    std::string_view preText = text;
    while (char32_t w = ToCodePoint(preText))
    {
        if (w < 0x20)
            continue;
        CharGlyph const* glyph = Glyph(w);
        if (glyph == nullptr)
            continue;
        textCount++;
    }
    if (textCount == 0)
        return nullptr;

    xxMeshPtr output = mesh;
    if (output == nullptr)
        output = xxMesh::Create(false, 0, 1, 1);
    if (output == nullptr)
        return nullptr;

    // Shadow
    int meshCount = textCount;
    if (shadow > 0.0f)
        meshCount *= 2;

    // Create indices
    if (output->IndexCount != meshCount * 6)
    {
        output->SetIndexCount(meshCount * 6);
        auto indices = (uint16_t*)output->Index;
        for (size_t i = 0, x = 1; i < meshCount; ++i)
        {
            (*indices++) = x + 0;
            (*indices++) = x + 1;
            (*indices++) = x + 2;
            (*indices++) = x + 0;
            (*indices++) = x + 2;
            (*indices++) = x + 3;
            x += 4;
        }
    }

    // Create Vertices
    output->SetVertexCount(1 + meshCount * 4);
    auto positions = output->GetPosition();
    auto colors = output->GetColor(0);
    auto textures = output->GetTexture(0);
    (*positions++) = { scale.x, scale.y, shadow };
    (*colors++) = 0;
    (*textures++) = xxVector2::ZERO;

    // Color
    uint32_t textColors[4];
    for (int i = 0; i < 4; ++i)
    {
        textColors[i] = xxVector4{ color[i].x, color[i].y, color[i].z, 0.5f }.ToInteger();
    }

    // Glyph
    xxVector2 rescale = scale / SIZE;
    float advanced = 0.0f;
    float height = scale.y;
    while (char32_t w = ToCodePoint(text))
    {
        if (w == '\n')
        {
            advanced = 0.0f;
            height += scale.y;
            continue;
        }
        if (w < 0x20)
            continue;
        CharGlyph const* glyph = Glyph(w);
        if (glyph == nullptr)
            continue;
        xxVector2 rectLT = xxVector2{ float(glyph->rectLT.x), float(glyph->rectLT.y) } * rescale;
        xxVector2 rectRB = xxVector2{ float(glyph->rectRB.x), float(glyph->rectRB.y) } * rescale;
        (*positions++) = { advanced + rectLT.x, height + rectLT.y, 1.0f };
        (*positions++) = { advanced + rectRB.x, height + rectLT.y, 1.0f };
        (*positions++) = { advanced + rectRB.x, height + rectRB.y, 1.0f };
        (*positions++) = { advanced + rectLT.x, height + rectRB.y, 1.0f };
        (*colors++) = textColors[0];
        (*colors++) = textColors[2];
        (*colors++) = textColors[3];
        (*colors++) = textColors[1];
        (*textures++) = { glyph->uvLT.x, glyph->uvLT.y };
        (*textures++) = { glyph->uvRB.x, glyph->uvLT.y };
        (*textures++) = { glyph->uvRB.x, glyph->uvRB.y };
        (*textures++) = { glyph->uvLT.x, glyph->uvRB.y };
        advanced += glyph->advance * rescale.x;
    }
    if (shadow > 0.0f)
    {
        auto firstPositions = output->GetPosition() + 1;
        auto firstColors = output->GetColor(0) + 1;
        auto firstTextures = output->GetTexture(0) + 1;
        uint32_t colorBack = xxVector4{ 0.0f, 0.0f, 0.0f, shadow * 0.5f + 0.5f }.ToInteger();
        for (int i = 0, count = textCount * 4; i < count; ++i)
        {
            (*positions++) = (*firstPositions++);
            std::swap((*colors++) = colorBack, (*firstColors++));
            (*textures++) = (*firstTextures++);
        }
    }

    return output;
}
//------------------------------------------------------------------------------
xxMeshPtr Font::MeshColor(xxMeshPtr const& mesh, xxMatrix3x4 const color)
{
    if (mesh == nullptr)
        return nullptr;

    uint32_t textColors[4];
    for (int i = 0; i < 4; ++i)
    {
        textColors[i] = xxVector4{ color[i].x, color[i].y, color[i].z, 0.5f }.ToInteger();
    }

    mesh->SetVertexCount(mesh->VertexCount);
    int textCount = (mesh->VertexCount - 1) / 4;
    auto positions = mesh->GetPosition();
    auto colors = mesh->GetColor(0);
    xxVector3& payload = (*positions++);
    (*colors++);

    float shadow = payload.z;
    if (shadow > 0.0f)
    {
        textCount /= 2;
        uint32_t colorBack = xxVector4{ 0.0f, 0.0f, 0.0f, shadow * 0.5f + 0.5f }.ToInteger();
        for (int i = 0; i < textCount; ++i)
        {
            (*colors++) = colorBack;
            (*colors++) = colorBack;
            (*colors++) = colorBack;
            (*colors++) = colorBack;
        }
    }
    for (int i = 0; i < textCount; ++i)
    {
        (*colors++) = textColors[0];
        (*colors++) = textColors[2];
        (*colors++) = textColors[3];
        (*colors++) = textColors[1];
    }

    return mesh;
}
//------------------------------------------------------------------------------
xxMeshPtr Font::MeshScale(xxMeshPtr const& mesh, xxVector2 const& scale)
{
    if (mesh == nullptr)
        return nullptr;

    auto positions = mesh->GetPosition();
    xxVector3& payload = (*positions++);
    if (payload.xy == scale || scale.x <= 0.0f || scale.y <= 0.0f)
        return mesh;

    int vertexCount = (mesh->VertexCount - 1);
    xxVector2 rescale = scale / payload.xy;
    payload.xy = scale;
    for (int i = 0; i < vertexCount; ++i)
    {
        xxVector3& position = (*positions++);
        position.xy *= rescale;
    }
    mesh->SetVertexCount(1 + vertexCount);

    return mesh;
}
//------------------------------------------------------------------------------
xxMeshPtr Font::MeshShadow(xxMeshPtr const& mesh, float shadow)
{
    if (mesh == nullptr)
        return nullptr;

    auto positions = mesh->GetPosition();
    xxVector3& payload = (*positions++);
    if (payload.z == shadow)
        return mesh;

    int vertexCount = (mesh->VertexCount - 1);
    float oldShadow = payload.z;
    payload.z = shadow;
    if (oldShadow > 0.0f && shadow == 0.0f)
    {
        vertexCount /= 2;

        auto firstColors = mesh->GetColor(0) + 1;
        auto secondColors = firstColors + vertexCount;
        for (int i = 0; i < vertexCount; ++i)
        {
            (*firstColors++) = (*secondColors++);
        }

        mesh->SetIndexCount(vertexCount * 6 / 4);
        mesh->SetVertexCount(1 + vertexCount);
    }
    else if (oldShadow == 0.0f && shadow > 0.0f)
    {
        mesh->SetIndexCount(vertexCount * 2 * 6 / 4);
        mesh->SetVertexCount(1 + vertexCount * 2);

        int meshCount = vertexCount / 4 * 2;
        auto indices = (uint16_t*)mesh->Index;
        for (size_t i = 0, x = 1; i < meshCount; ++i)
        {
            (*indices++) = x + 0;
            (*indices++) = x + 1;
            (*indices++) = x + 2;
            (*indices++) = x + 0;
            (*indices++) = x + 2;
            (*indices++) = x + 3;
            x += 4;
        }

        auto positions = mesh->GetPosition() + 1 + vertexCount;
        auto colors = mesh->GetColor(0) + 1 + vertexCount;
        auto textures = mesh->GetTexture(0) + 1 + vertexCount;
        auto firstPositions = mesh->GetPosition() + 1;
        auto firstColors = mesh->GetColor(0) + 1;
        auto firstTextures = mesh->GetTexture(0) + 1;
        uint32_t colorBack = xxVector4{ 0.0f, 0.0f, 0.0f, shadow * 0.5f + 0.5f }.ToInteger();
        for (int i = 0; i < vertexCount; ++i)
        {
            (*positions++) = (*firstPositions++);
            std::swap((*colors++) = colorBack, (*firstColors++));
            (*textures++) = (*firstTextures++);
        }
    }
    else if (oldShadow > 0.0f && shadow > 0.0f)
    {
        mesh->SetVertexCount(1 + vertexCount);

        int count = vertexCount / 2;
        uint32_t colorBack = xxVector4{ 0.0f, 0.0f, 0.0f, shadow * 0.5f + 0.5f }.ToInteger();
        auto colors = mesh->GetColor(0) + 1;
        for (int i = 0; i < count; ++i)
        {
            (*colors++) = colorBack;
        }
    }

    return mesh;
}
//------------------------------------------------------------------------------
uint32_t Font::ToCodePoint(std::string_view& text)
{
    char32_t w = 0;
    while (text.length())
    {
        uint8_t c = text[0];
        if (c >= 0b11110000)
        {
            if (w) break;
            w = (c & 0b00000111);
        }
        else if (c >= 0b11100000)
        {
            if (w) break;
            w = (c & 0b00001111);
        }
        else if (c >= 0b11000000)
        {
            if (w) break;
            w = (c & 0b00011111);
        }
        else if (c >= 0b10000000)
        {
            w = w << 6;
            w = (c & 0b00111111) | w;
        }
        else
        {
            if (w) break;
            w = c;
        }
        text.remove_prefix(1);
    }
    return w;
}
//------------------------------------------------------------------------------
std::string Font::ToUTF8(std::u32string const& text)
{
    std::string output;
    for (char32_t w : text)
    {
        if (w <= 0x7F)
        {
            output.push_back(((w >>  0) & 0b01111111));
        }
        else if (w <= 0x7FF)
        {
            output.push_back(((w >>  6) & 0b00011111) | 0b11000000);
            output.push_back(((w >>  0) & 0b00111111) | 0b10000000);
        }
        else if (w <= 0xFFFF)
        {
            output.push_back(((w >> 12) & 0b00001111) | 0b11100000);
            output.push_back(((w >>  6) & 0b00111111) | 0b10000000);
            output.push_back(((w >>  0) & 0b00111111) | 0b10000000);
        }
        else if (w <= 0x10FFFF)
        {
            output.push_back(((w >> 18) & 0b00000111) | 0b11110000);
            output.push_back(((w >> 12) & 0b00111111) | 0b10000000);
            output.push_back(((w >>  6) & 0b00111111) | 0b10000000);
            output.push_back(((w >>  0) & 0b00111111) | 0b10000000);
        }
    }
    return output;
}
//------------------------------------------------------------------------------
std::u32string Font::ToUTF32(std::string_view text)
{
    std::u32string output;
    while (char32_t w = ToCodePoint(text))
        output.push_back(w);
    return output;
}
//==============================================================================
}   // namespace MiniGUI
#endif
