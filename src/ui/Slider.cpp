#include "Slider.hpp"

#include "Canvas.hpp"

#include <algorithm>

namespace flachead::ui
{
Slider::Slider()
{
    SetFocusable(true);
}

void Slider::SetRange(float min, float max)
{
    m_Min = min;
    m_Max = std::max(min, max);
    m_Value = std::clamp(m_Value, m_Min, m_Max);
}

void Slider::SetValue(float value)
{
    const float clamped = std::clamp(value, m_Min, m_Max);
    if (clamped == m_Value)
    {
        return;
    }
    m_Value = clamped;
    if (m_OnChange)
    {
        m_OnChange(m_Value);
    }
}

Vec2 Slider::PreferredSize() const
{
    return Vec2{160.0f, 26.0f};
}

void Slider::SeekToPosition(float x)
{
    const Rect bounds = Bounds();
    const float range = std::max(1.0f, bounds.size.x - 20.0f);
    const float t = std::clamp((x - bounds.position.x - 10.0f) / range, 0.0f, 1.0f);
    SetValue(m_Min + (m_Max - m_Min) * t);
}

void Slider::OnDraw(Canvas& canvas)
{
    const Rect bounds = Bounds();
    const float trackY = bounds.position.y + bounds.size.y * 0.5f - 2.0f;
    canvas.FillRoundedRect(Rect{bounds.position.x, trackY, bounds.size.x, 4.0f},
                           2.0f, m_TrackColor);

    const float range = std::max(1.0f, bounds.size.x - 20.0f);
    const float t = (m_Value - m_Min) / (m_Max - m_Min);
    const float fillW = 10.0f + range * t;
    if (fillW > 1.0f)
    {
        canvas.FillRoundedRect(Rect{bounds.position.x, trackY, fillW, 4.0f}, 2.0f, m_Color);
    }

    const float thumbX = bounds.position.x + 10.0f + range * t;
    const float thumbY = bounds.position.y + bounds.size.y * 0.5f;
    const float thumbR = (Focused() || Pressed()) ? 9.0f : 7.0f;
    canvas.FillCircle(thumbX, thumbY, thumbR, m_Color);
    if (Focused())
    {
        canvas.DrawCircle(thumbX, thumbY, thumbR + 3.0f, m_Color);
    }
}

bool Slider::OnHandleEvent(const flachead::input::InputEvent& event)
{
    switch (event.action)
    {
        case flachead::input::InputAction::Press:
            m_Dragging = true;
            SeekToPosition(event.position.x);
            return true;
        case flachead::input::InputAction::DragMove:
            if (m_Dragging)
            {
                SeekToPosition(event.position.x);
                return true;
            }
            break;
        case flachead::input::InputAction::Release:
            m_Dragging = false;
            return true;
        case flachead::input::InputAction::KeyDown:
            if (event.key == SDLK_LEFT || event.key == SDLK_RIGHT)
            {
                const float step = (m_Max - m_Min) * 0.02f;
                SetValue(m_Value + (event.key == SDLK_RIGHT ? step : -step));
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}
} // namespace flachead::ui
