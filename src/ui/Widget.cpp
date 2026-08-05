#include "Widget.hpp"

#include <algorithm>
#include <utility>

namespace flachead::ui
{
Widget::Widget() = default;

void Widget::SetEnabled(bool enabled)
{
    m_Enabled = enabled;
    if (!enabled)
    {
        m_Focused = false;
        m_Pressed = false;
        m_Hovered = false;
    }
}

void Widget::SetSelected(bool selected)
{
    if (m_Selected != selected)
    {
        m_Selected = selected;
    }
}

Rect Widget::GlobalBounds() const
{
    Rect result = m_Bounds;
    const Widget* current = this;
    while (current->m_Parent)
    {
        current = current->m_Parent;
        result.position.x += current->m_Bounds.position.x;
        result.position.y += current->m_Bounds.position.y;
    }
    return result;
}

bool Widget::ContainsPoint(const Vec2& point) const
{
    return point.x >= m_Bounds.position.x && point.x <= m_Bounds.position.x + m_Bounds.size.x &&
           point.y >= m_Bounds.position.y && point.y <= m_Bounds.position.y + m_Bounds.size.y;
}

Widget* Widget::ChildAtPoint(const Vec2& point)
{
    for (auto it = m_Children.rbegin(); it != m_Children.rend(); ++it)
    {
        Widget* child = it->get();
        if (child->Visible() && child->ContainsPoint(point))
        {
            return child;
        }
    }
    return nullptr;
}

void Widget::AddChild(std::unique_ptr<Widget> child)
{
    child->m_Parent = this;
    m_Children.push_back(std::move(child));
}

void Widget::RemoveChild(std::string_view id)
{
    m_Children.erase(std::remove_if(m_Children.begin(), m_Children.end(),
                                    [id](const std::unique_ptr<Widget>& child) {
                                        return child->Id() == id;
                                    }),
                     m_Children.end());
}

void Widget::RemoveAllChildren()
{
    m_Children.clear();
}

Widget* Widget::FindById(std::string_view id)
{
    for (const auto& child : m_Children)
    {
        if (child->Id() == id)
        {
            return child.get();
        }
        if (Widget* found = child->FindById(id))
        {
            return found;
        }
    }
    return nullptr;
}

const Widget* Widget::FindById(std::string_view id) const
{
    for (const auto& child : m_Children)
    {
        if (child->Id() == id)
        {
            return child.get();
        }
        if (const Widget* found = child->FindById(id))
        {
            return found;
        }
    }
    return nullptr;
}

void Widget::Layout(const Rect& bounds)
{
    m_Bounds = bounds;
    if (m_Children.empty() || !m_Layout)
    {
        return;
    }

    std::vector<Vec2> preferred;
    preferred.reserve(m_Children.size());
    for (const auto& child : m_Children)
    {
        preferred.push_back(child->Visible() ? child->PreferredSize() : Vec2{0.0f, 0.0f});
    }

    const flachead::layout::LayoutResult result = m_Layout->Layout(preferred, bounds.size);
    for (std::size_t i = 0; i < m_Children.size(); ++i)
    {
        if (i >= result.rects.size())
        {
            break;
        }
        const Rect& childBounds = result.rects[i];
        m_Children[i]->Layout(Rect{childBounds.position.x + bounds.position.x,
                                   childBounds.position.y + bounds.position.y,
                                   childBounds.size.x, childBounds.size.y});
    }
}

void Widget::Update(float deltaSeconds)
{
    for (const auto& child : m_Children)
    {
        child->Update(deltaSeconds);
    }
    OnUpdate(deltaSeconds);
}

void Widget::Draw(Canvas& canvas)
{
    if (!m_Visible)
    {
        return;
    }
    for (const auto& child : m_Children)
    {
        child->Draw(canvas);
    }
    OnDraw(canvas);
}

bool Widget::HandleEvent(const flachead::input::InputEvent& event)
{
    if (!m_Visible || !m_Enabled)
    {
        return false;
    }

    switch (event.action)
    {
        case flachead::input::InputAction::Tap:
        case flachead::input::InputAction::Press:
        {
            if (Widget* child = ChildAtPoint(event.position))
            {
                if (child->HandleEvent(event))
                {
                    return true;
                }
            }
            break;
        }
        case flachead::input::InputAction::DragMove:
        case flachead::input::InputAction::DragEnd:
        case flachead::input::InputAction::Swipe:
        case flachead::input::InputAction::DoubleTap:
        case flachead::input::InputAction::Hold:
        case flachead::input::InputAction::KeyDown:
        case flachead::input::InputAction::KeyUp:
        {
            for (const auto& child : m_Children)
            {
                if (child->Focused() && child->HandleEvent(event))
                {
                    return true;
                }
            }
            break;
        }
        case flachead::input::InputAction::Release:
            break;
    }
    return OnHandleEvent(event);
}

void Widget::NotifyFocusGained()
{
    if (m_Focused)
    {
        return;
    }
    m_Focused = true;
    OnFocusGained();
}

void Widget::NotifyFocusLost()
{
    if (!m_Focused)
    {
        return;
    }
    m_Focused = false;
    m_Pressed = false;
    m_Hovered = false;
    OnFocusLost();
}

void Widget::SetPressed(bool pressed)
{
    m_Pressed = pressed;
}

void Widget::SetHovered(bool hovered)
{
    m_Hovered = hovered;
}
} // namespace flachead::ui
