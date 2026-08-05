#include "ScrollBar.hpp"

#include "Canvas.hpp"

#include <algorithm>

namespace flachead::ui
{
ScrollBar::ScrollBar()
{
    SetFocusable(false);
}

void ScrollBar::SetValue(float value)
{
    m_Value = std::clamp(value, 0.0f, 1.0f);
}

void ScrollBar::SetViewportFraction(float fraction)
{
    m_Fraction = std::clamp(fraction, 0.05f, 1.0f);
}

Vec2 ScrollBar::PreferredSize() const
{
    return Vec2{4.0f, 120.0f};
}

void ScrollBar::OnDraw(Canvas& canvas)
{
    const Rect bounds = Bounds();
    const float thumbH = std::max(20.0f, bounds.size.y * m_Fraction);
    const float maxOffset = std::max(0.0f, bounds.size.y - thumbH);
    const float y = bounds.position.y + m_Value * maxOffset;
    canvas.FillRoundedRect(Rect{bounds.position.x, bounds.position.y,
                                bounds.size.x, bounds.size.y},
                           2.0f, Color{25, 28, 38, 255});
    canvas.FillRoundedRect(Rect{bounds.position.x, y, bounds.size.x, thumbH}, 2.0f, m_Color);
}
} // namespace flachead::ui
