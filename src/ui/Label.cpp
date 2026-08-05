#include "Label.hpp"

#include "Canvas.hpp"

namespace flachead::ui
{
Label::Label()
{
    SetFocusable(false);
}

void Label::SetText(std::string_view text)
{
    m_Text = text;
}

Vec2 Label::PreferredSize() const
{
    if (m_Text.empty())
    {
        return {};
    }
    return Vec2{static_cast<float>(m_Text.size()) * m_FontSize * 0.55f + m_FontSize,
                m_FontSize + 6.0f};
}

void Label::OnDraw(Canvas& canvas)
{
    if (m_Text.empty())
    {
        return;
    }
    const Rect bounds = Bounds();
    Rect drawRect = bounds;
    switch (m_Align)
    {
        case Align::Left:
            break;
        case Align::Center:
            drawRect.position.x = bounds.position.x + (bounds.size.x - m_FontSize * 0.55f *
                                                          static_cast<float>(m_Text.size())) * 0.5f;
            break;
        case Align::Right:
            drawRect.position.x = bounds.position.x + bounds.size.x - m_FontSize * 0.55f *
                                                            static_cast<float>(m_Text.size());
            break;
    }
    const Color color = m_Color;
    canvas.DrawText(drawRect, m_Text, color, m_FontSize);
}
} // namespace flachead::ui
