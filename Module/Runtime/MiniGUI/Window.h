//==============================================================================
// Minamoto : Window Header
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#pragma once

#if HAVE_MINIGUI

#include "Runtime.h"
#include <xxGraphicPlus/xxNode.h>

namespace MiniGUI
{
class RuntimeAPI Window : public xxNode
{
public:
    enum Type
    {
        TEXT                = 0,
        TEXT_COLOR          = 1,
        TEXT_SCALE          = 2,
        TEXT_SHADOW         = 3,
    };

    template<typename C, typename T>
    void                    SetType(Type type, T const& text);
    template<typename C, typename T>
    T                       GetType(Type type, T const& fallback) const;

    auto                    begin() { return ((std::vector<WindowPtr>&)m_children).begin(); }
    auto                    begin() const { return ((std::vector<WindowPtr>&)m_children).begin(); }
    auto                    end() { return ((std::vector<WindowPtr>&)m_children).end(); }
    auto                    end() const { return ((std::vector<WindowPtr>&)m_children).end(); }

    enum
    {
        WINDOW_CLASS            = 0x00000010,
        UPDATE_TEXT             = 0x00000020,
    };

public:
    std::string_view        GetText() const;
    std::array<xxVector3, 4>GetTextColor() const;
    float                   GetTextScale() const;
    float                   GetTextShadow() const;
    void                    SetText(std::string_view const& text);
    void                    SetTextColor(std::array<xxVector3, 4> const& color);
    void                    SetTextScale(float size);
    void                    SetTextShadow(float size);
    void                    UpdateText();

    xxVector2               GetScale() const { return { LocalMatrix[0].x, LocalMatrix[1].y }; }
    xxVector2 const&        GetOffset() const { return LocalMatrix[3].xy; }
    void                    SetScale(xxVector2 const& scale);
    void                    SetOffset(xxVector2 const& offset) { LocalMatrix[3].xy = offset; }

    xxVector2               GetWorldScale() const { return { WorldMatrix[0].x, WorldMatrix[1].y }; }
    xxVector2 const&        GetWorldOffset() const { return WorldMatrix[3].xy; }

    WindowPtr const&        GetParent() const;
    WindowPtr const&        GetChild(size_t index) const;

    static bool             Traversal(WindowPtr const& window, std::function<bool(WindowPtr const&)> callback);

    static xxNodePtr        Create();

    virtual void            BinaryRead(xxBinary& binary);
    virtual void            BinaryWrite(xxBinary& binary) const;

    static WindowPtr const& Cast(xxNodePtr const& node);

    static void             Update(WindowPtr const& window, float time, float width, float height);

public:
    static xxVector2        ScreenSize;
    static xxVector2        ScreenInvSize;
};
}   // namespace MiniGUI
#endif
