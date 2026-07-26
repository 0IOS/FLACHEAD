#include "Widget.hpp"

namespace flachead::ui
{

void Widget::SetId(std::string_view id)
{
    m_Id = id;
}

std::string_view Widget::Id() const
{
    return m_Id;
}

void Widget::SetEnabled(bool enabled)
{
    m_Enabled = enabled;
}

bool Widget::Enabled() const
{
    return m_Enabled;
}

void Widget::SetBounds(const Rect& bounds)
{
    m_Bounds = bounds;
}

const Rect& Widget::Bounds() const
{
    return m_Bounds;
}

void Widget::SetVisible(bool visible)
{
    m_Visible = visible;
}

bool Widget::Visible() const
{
    return m_Visible;
}

}