#include "Button.hpp"

#include "Canvas.hpp"

#include <algorithm>
#include <cmath>

namespace flachead::ui
{
namespace
{
constexpr float kPressTarget = 0.22f;
} // namespace

Button::Button()
{
    SetFocusable(true);
}

void Button::SetText(std::string_view text)
{
    m_Text = text;
}

void Button::Click()
{
    if (m_ClickHandler)
    {
        m_ClickHandler();
    }
}

Vec2 Button::PreferredSize() const
{
    const float textW = m_Text.empty() ? 0.0f : static_cast<float>(m_Text.size()) * 15.0f;
    return Vec2{std::max(textW + 32.0f, 96.0f), 44.0f};
}

void Button::OnUpdate(float deltaSeconds)
{
    const float target = Pressed() ? kPressTarget : 0.0f;
    const float rate = Pressed() ? 14.0f : 10.0f;
    m_PressAmount += (target - m_PressAmount) * std::min(1.0f, deltaSeconds * rate);
    if (std::abs(m_PressAmount - target) < 0.001f)
    {
        m_PressAmount = target;
    }
}

void Button::OnDraw(Canvas& canvas)
{
    const Rect bounds = Bounds();
    const float radius = std::min(12.0f, bounds.size.y * 0.35f);

    Color background = Disabled() ? Color{30, 32, 40, 255} : m_Background;
    if (Selected() || Focused())
    {
        const uint8_t blend = static_cast<uint8_t>(40 * (1.0f - m_PressAmount));
        background = Color{static_cast<uint8_t>(m_Accent.r * 0.35f + background.r * 0.65f + blend),
                           static_cast<uint8_t>(m_Accent.g * 0.35f + background.g * 0.65f + blend),
                           static_cast<uint8_t>(m_Accent.b * 0.35f + background.b * 0.65f + blend),
                           255};
    }

    canvas.FillRoundedRect(bounds, radius, background);

    if (Focused() && !Disabled())
    {
        canvas.DrawRoundedRect(Rect{bounds.position.x + 2.0f, bounds.position.y + 2.0f,
                                    bounds.size.x - 4.0f, bounds.size.y - 4.0f},
                               radius, m_Accent);
    }

    if (Pressed())
    {
        canvas.FillRoundedRect(bounds, radius, Color{0, 0, 0, static_cast<uint8_t>(70 * m_PressAmount)});
    }

    const Color textColor = Disabled() ? Color{120, 120, 128, 255} : Color{235, 238, 245, 255};
    canvas.DrawTextCentered(Rect{bounds.position.x + 4.0f, bounds.position.y,
                                 bounds.size.x - 8.0f, bounds.size.y},
                            m_Text, textColor, 18.0f);
}

bool Button::OnHandleEvent(const flachead::input::InputEvent& event)
{
    switch (event.action)
    {
        case flachead::input::InputAction::Tap:
            Click();
            return true;
        case flachead::input::InputAction::KeyDown:
            if (event.key == SDLK_RETURN || event.key == SDLK_KP_ENTER || event.key == SDLK_SPACE)
            {
                Click();
                return true;
            }
            break;
        case flachead::input::InputAction::Press:
            if (ContainsPoint(event.position))
            {
                SetPressed(true);
                if (m_PressCallback)
                {
                    m_PressCallback(true);
                }
                return true;
            }
            break;
        case flachead::input::InputAction::Release:
            if (Pressed())
            {
                SetPressed(false);
                if (m_PressCallback)
                {
                    m_PressCallback(false);
                }
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

void Button::OnFocusGained()
{
    m_Hovered = true;
}

void Button::OnFocusLost()
{
    m_Hovered = false;
    SetPressed(false);
}
} // namespace flachead::ui
