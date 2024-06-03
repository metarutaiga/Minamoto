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
static inline std::span<char> FromColor4(std::array<xxVector3, 4> const& color)
{
    if (color[0] == color[1] && color[2] == color[3])
    {
        if (color[1] == color[2])
        {
            return { (char*)color.data(), (char*)(color.data() + 1) };
        }
        return { (char*)color.data(), (char*)(color.data() + 2) };
    }
    return { (char*)color.data(), (char*)(color.data() + 4) };
}
//------------------------------------------------------------------------------
static inline std::array<xxVector3, 4> ToColor4(std::span<char> span)
{
    auto* color = (xxVector3*)span.data();
    if (span.size() < sizeof(xxVector3) * 1)
    {
        static std::array<xxVector3, 4> const white = { xxVector3::WHITE, xxVector3::WHITE, xxVector3::WHITE, xxVector3::WHITE };
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
std::array<xxVector3, 4> Window::GetTextColor() const
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
void Window::SetTextColor(std::array<xxVector3, 4> const& color)
{
    if (GetTextColor() == color)
        return;
    SetType<ArrayModifier>(TEXT_COLOR, FromColor4(color));
    Flags |= UPDATE_TEXT;
}
//------------------------------------------------------------------------------
void Window::SetTextScale(float scale)
{
    if (GetTextScale() == scale)
        return;
    SetType<FloatModifier>(TEXT_SCALE, scale);
    Flags |= UPDATE_TEXT;
}
//------------------------------------------------------------------------------
void Window::SetTextShadow(float shadow)
{
    if (GetTextShadow() == shadow)
        return;
    SetType<FloatModifier>(TEXT_SHADOW, shadow);
    Flags |= UPDATE_TEXT;
}
//------------------------------------------------------------------------------
void Window::UpdateText()
{
    float scale = GetTextScale();
    float scaleX = scale * ScreenInvSize.x / LocalMatrix[0].x;
    float scaleY = scale * ScreenInvSize.y / LocalMatrix[1].y;
    Material = Font::Material();
    Mesh = Font::Mesh(Mesh, GetText(), GetTextColor().data(), scaleX, scaleY, GetTextShadow());
}
//------------------------------------------------------------------------------
void Window::SetScale(xxVector2 const& scale)
{
    LocalMatrix[0].x = scale.x;
    LocalMatrix[1].y = scale.y;
    Flags |= UPDATE_TEXT;
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
bool Window::Traversal(WindowPtr const& window, std::function<bool(WindowPtr const&)> callback)
{
    return xxNode::Traversal(window, (std::function<bool(xxNodePtr const&)>&)callback);
}
//------------------------------------------------------------------------------
xxNodePtr Window::Create()
{
    xxNodePtr node = xxNode::Create();
    if (node == nullptr)
        return nullptr;
    Window* window = (Window*)node.get();

    window->Flags = WINDOW_CLASS;
    window->SetScale(xxVector2::ONE);
    window->SetOffset(xxVector2::ZERO);
    return node;
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
void Window::Update(WindowPtr const& window, float time, float width, float height)
{
    bool resize = false;
    if (ScreenSize.x != width || ScreenSize.y != height)
    {
        ScreenSize.x = width;
        ScreenSize.y = height;
        ScreenInvSize.x = 1.0f / width;
        ScreenInvSize.y = 1.0f / height;
        resize = true;
    }

    auto callback = [](WindowPtr const& window)
    {
        if (window->Flags & UPDATE_TEXT)
        {
            window->Flags &= ~UPDATE_TEXT;
            window->UpdateText();
        }
        window->UpdateMatrix();
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
