//==============================================================================
// Minamoto : Window Source
//
// Copyright (c) 2019-2024 TAiGA
// https://github.com/metarutaiga/minamoto
//==============================================================================
#include "Runtime.h"
#include <xxGraphicPlus/xxMaterial.h>
#include "Modifier/FloatModifier.h"
#include "Modifier/ArrayModifier.h"
#include "Modifier/StringModifier.h"
#include "Tools/NodeTools.h"
#include "Font.h"
#include "Window.h"

#if HAVE_MINIGUI
namespace MiniGUI
{
xxVector2 Window::ScreenSize;
xxVector2 Window::ScreenInvSize;
//==============================================================================
//  Template
//==============================================================================
template<typename C, typename T>
void Window::SetType(Type type, T const& value)
{
    if (Modifiers.size() <= type)
        Modifiers.resize(type + 1);
    if (Modifiers[type].modifier == nullptr)
        Modifiers[type].modifier = { C::Create() };
    auto ptr = static_cast<C*>(Modifiers[type].modifier.get());
    ptr->Set(value);
}
//------------------------------------------------------------------------------
template<typename C, typename T>
T Window::GetType(Type type, T const& fallback) const
{
    if (Modifiers.size() <= type)
        return fallback;
    if (Modifiers[type].modifier == nullptr)
        return fallback;
    auto ptr = static_cast<C*>(Modifiers[type].modifier.get());
    return ptr->Get();
}
//------------------------------------------------------------------------------
static inline std::span<char> FromColor4(xxMatrix3x4 const& color)
{
    if (color[0] == color[1] && color[2] == color[3])
    {
        if (color[1] == color[2])
        {
            return { (char*)color.v, (char*)(color.v + 1) };
        }
        return { (char*)color.v, (char*)(color.v + 2) };
    }
    return { (char*)color.v, (char*)(color.v + 4) };
}
//------------------------------------------------------------------------------
static inline xxMatrix3x4 ToColor4(std::span<char> span)
{
    auto* color = (xxVector3*)span.data();
    if (span.size() < sizeof(xxVector3) * 1)
    {
        static xxMatrix3x4 const white = { xxVector3::WHITE, xxVector3::WHITE, xxVector3::WHITE, xxVector3::WHITE };
        return white;
    }
    if (span.size() < sizeof(xxVector3) * 2)
    {
        return { color[0], color[0], color[0], color[0] };
    }
    if (span.size() < sizeof(xxVector3) * 4)
    {
        return { color[0], color[1], color[0], color[1] };
    }
    return { color[0], color[1], color[2], color[3] };
}
//==============================================================================
//  Property
//==============================================================================
std::string_view Window::GetText() const
{
    return GetType<StringModifier, std::string_view>(TEXT, "");
}
//------------------------------------------------------------------------------
xxMatrix3x4 Window::GetTextColor() const
{
    return ToColor4(GetType<ArrayModifier, std::span<char>>(TEXT_COLOR, {}));
}
//------------------------------------------------------------------------------
float Window::GetTextScale() const
{
    return GetType<FloatModifier, float>(TEXT_SCALE, 16.0f);
}
//------------------------------------------------------------------------------
float Window::GetTextShadow() const
{
    return GetType<FloatModifier, float>(TEXT_SHADOW, 0.0f);
}
//------------------------------------------------------------------------------
void Window::SetText(std::string_view const& text)
{
    if (GetText() == text)
        return;
    SetType<StringModifier>(TEXT, text);
    Flags |= UPDATE_TEXT;
}
//------------------------------------------------------------------------------
void Window::SetTextColor(xxMatrix3x4 const& color)
{
    if (GetTextColor() == color)
        return;
    SetType<ArrayModifier>(TEXT_COLOR, FromColor4(color));
    Flags |= UPDATE_TEXT_COLOR;
}
//------------------------------------------------------------------------------
void Window::SetTextScale(float scale)
{
    if (GetTextScale() == scale)
        return;
    SetType<FloatModifier>(TEXT_SCALE, scale);
    Flags |= UPDATE_TEXT_SCALE;
}
//------------------------------------------------------------------------------
void Window::SetTextShadow(float shadow)
{
    if (GetTextShadow() == shadow)
        return;
    SetType<FloatModifier>(TEXT_SHADOW, shadow);
    Flags |= UPDATE_TEXT_SHADOW;
}
//------------------------------------------------------------------------------
void Window::UpdateText()
{
    if (Flags & UPDATE_TEXT)
    {
        Flags &= ~UPDATE_TEXT_FLAGS;
        xxVector2 scale = ScreenInvSize / GetScale() * GetTextScale();
        Material = Font::Material();
        Mesh = Font::Mesh(Mesh, GetText(), GetTextColor(), scale, GetTextShadow());
        return;
    }
    if (Flags & UPDATE_TEXT_COLOR)
    {
        Flags &= ~UPDATE_TEXT_COLOR;
        Mesh = Font::MeshColor(Mesh, GetTextColor());
    }
    if (Flags & UPDATE_TEXT_SCALE)
    {
        Flags &= ~UPDATE_TEXT_SCALE;
        xxVector2 scale = ScreenInvSize / GetScale() * GetTextScale();
        Mesh = Font::MeshScale(Mesh, scale);
    }
    if (Flags & UPDATE_TEXT_SHADOW)
    {
        Flags &= ~UPDATE_TEXT_SHADOW;
        Mesh = Font::MeshShadow(Mesh, GetTextShadow());
    }
}
//------------------------------------------------------------------------------
void Window::SetScale(xxVector2 const& scale)
{
    LocalMatrix[0].x = scale.x;
    LocalMatrix[1].y = scale.y;
    Flags |= UPDATE_TEXT;

    // Update flag
    Flags |= UPDATE_NEED;
    WindowPtr parent = GetParent();
    while (parent)
    {
        parent->Flags |= UPDATE_NEED;
        parent = parent->GetParent();
    }
}
//------------------------------------------------------------------------------
void Window::SetOffset(xxVector2 const& offset)
{
    LocalMatrix[3].xy = offset;

    // Update flag
    Flags |= UPDATE_NEED;
    WindowPtr parent = GetParent();
    while (parent)
    {
        parent->Flags |= UPDATE_NEED;
        parent = parent->GetParent();
    }
}
//==============================================================================
//  Node
//==============================================================================
WindowPtr const& Window::GetParent() const
{
    if (m_parent.expired())
    {
        static WindowPtr empty;
        return empty;
    }
    WindowPtr const& parent = (WindowPtr&)m_parent;
    if ((parent->Flags & WINDOW_CLASS) == 0)
    {
        static WindowPtr empty;
        return empty;
    }
    return parent;
}
//------------------------------------------------------------------------------
WindowPtr const& Window::GetChild(size_t index) const
{
    if (index >= m_children.size())
    {
        static WindowPtr empty;
        return empty;
    }
    return (WindowPtr&)m_children[index];
}
//------------------------------------------------------------------------------
std::vector<WindowPtr>& Window::GetChildren() const
{
    return (std::vector<WindowPtr>&)m_children;
}
//------------------------------------------------------------------------------
bool Window::Traversal(WindowPtr const& window, std::function<bool(WindowPtr const&)> callback)
{
    return xxNode::Traversal(window, (std::function<bool(xxNodePtr const&)>&)callback);
}
//------------------------------------------------------------------------------
WindowPtr Window::Create()
{
    xxNodePtr node = xxNode::Create();
    if (node == nullptr)
        return nullptr;
    node->Flags = WINDOW_CLASS;

    WindowPtr window = Cast(node);
    window->SetScale(xxVector2::ONE);
    window->SetOffset(xxVector2::ZERO);
    return window;
}
//------------------------------------------------------------------------------
WindowPtr const& Window::Cast(xxNodePtr const& node)
{
    if (node == nullptr || (node->Flags & WINDOW_CLASS) == 0)
    {
        static WindowPtr empty;
        return empty;
    }
    static void* vtable = nullptr;
    if (vtable == nullptr)
    {
        Window window;
        vtable = *(void**)&window;
    }
    void** temp = (void**)node.get();
    temp[0] = vtable;
    return (WindowPtr&)node;
}
//==============================================================================
//  Update
//==============================================================================
void Window::Update(WindowPtr const& window, float time, xxVector2 const& screenSize)
{
    bool resize = false;
    if (ScreenSize != screenSize)
    {
        ScreenSize = screenSize;
        ScreenInvSize = { 1.0f / screenSize.x, 1.0f / screenSize.y };
        resize = true;
    }

    auto callback = [&](WindowPtr const& window)
    {
        if (resize)
        {
            window->Flags |= UPDATE_TEXT_SCALE;
        }
        if (window->Flags & UPDATE_TEXT_FLAGS)
        {
            window->UpdateText();
        }
        if (window->Flags & UPDATE_NEED)
        {
            window->Flags &= ~UPDATE_NEED;
            window->UpdateMatrix();
        }
        return true;
    };
    Traversal(window, callback);
}
//==============================================================================
//  Binary
//==============================================================================
void Window::BinaryRead(xxBinary& binary)
{
    xxNode::BinaryRead(binary);
    Material = nullptr;
    Mesh = nullptr;
}
//------------------------------------------------------------------------------
void Window::BinaryWrite(xxBinary& binary) const
{
    auto material = Material;
    auto mesh = Mesh;
    const_cast<xxMaterialPtr&>(Material) = nullptr;
    const_cast<xxMeshPtr&>(Mesh) = nullptr;
    xxNode::BinaryWrite(binary);
    const_cast<xxMaterialPtr&>(Material) = material;
    const_cast<xxMeshPtr&>(Mesh) = mesh;
}
//==============================================================================
}   // namespace MiniGUI
#endif
