#include "ProgressBar.hpp"

#include "Canvas.hpp"

#include <algorithm>

namespace flachead::ui
{
ProgressBar::ProgressBar()
{
    SetFocusable(false);
}

void ProgressBar::SetValue(float value)
{
    m_Value = std::clamp(value, 0.0f, 1.0f);
}

Vec2 ProgressBar::PreferredSize() const
{
    return Vec2{120.0f, m_Radius * 2.0f + 4.0f};
}

void ProgressBar::OnDraw(Canvas& canvas)
{
    const Rect bounds = Bounds();
    canvas.FillRoundedRect(bounds, m_Radius, m_TrackColor);
    if (m_Value <= 0.0f)
    {
        return;
    }
    const float fillW = std::max(0.0f, bounds.size.x * m_Value);
    if (fillW < 1.0f)
    {
        return;
    }
    canvas.FillRoundedRect(Rect{bounds.position.x, bounds.position.y, fillW, bounds.size.y},
                           m_Radius, m_Color);
}
} // namespace flachead::ui
