#include "TextField.hpp"

#include "Canvas.hpp"

#include <SDL3/SDL.h>

#include <cctype>
#include <functional>
#include <utility>

namespace flachead::ui
{
TextField::TextField() = default;

void TextField::SetQuery(std::string_view query)
{
    m_Query = std::string{query};
}

Vec2 TextField::PreferredSize() const
{
    return Vec2{260.0f, 32.0f};
}

void TextField::Append(char ch)
{
    m_Query += ch;
    if (m_ChangeHandler)
    {
        m_ChangeHandler();
    }
}

void TextField::Backspace()
{
    if (m_Query.empty())
    {
        return;
    }
    std::size_t count = 1;
    while (count <= m_Query.size() && (static_cast<unsigned char>(m_Query[m_Query.size() - count]) & 0xC0) == 0x80)
    {
        ++count;
    }
    m_Query.resize(m_Query.size() - count);
    if (m_ChangeHandler)
    {
        m_ChangeHandler();
    }
}

bool TextField::OnHandleEvent(const flachead::input::InputEvent& event)
{
    if (event.action != flachead::input::InputAction::KeyDown || event.source == flachead::input::InputSource::Touch)
    {
        return false;
    }
    const int key = static_cast<int>(event.key);
    if (key == SDLK_BACKSPACE)
    {
        Backspace();
        return true;
    }
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
    {
        if (m_SubmitHandler)
        {
            m_SubmitHandler();
        }
        return true;
    }
    if (key >= SDLK_A && key <= SDLK_Z)
    {
        Append(static_cast<char>(key));
        return true;
    }
    if (key >= SDLK_0 && key <= SDLK_9)
    {
        Append(static_cast<char>(key));
        return true;
    }
    if (key == SDLK_SPACE)
    {
        Append(' ');
        return true;
    }
    return false;
}

void TextField::OnFocusGained()
{
    if (m_ChangeHandler)
    {
        m_ChangeHandler();
    }
}

void TextField::OnFocusLost()
{
}

void TextField::OnDraw(Canvas& canvas)
{
    const Rect bounds = Bounds();
    canvas.FillRoundedRect(bounds, 8.0f, m_Background);
    canvas.DrawRoundedRect(bounds, 8.0f, Color{255, 255, 255, 20});

    const Rect textBounds{bounds.position.x + 8.0f, bounds.position.y + 2.0f,
                          bounds.size.x - 16.0f, bounds.size.y - 4.0f};
    if (m_Query.empty())
    {
        canvas.DrawText(textBounds, m_Placeholder, m_PlaceholderColor, 14.0f);
    }
    else
    {
        canvas.DrawText(textBounds, m_Query, m_Color, 14.0f);
    }
    if (Focused())
    {
        const float caretX = bounds.position.x + 8.0f + (bounds.size.x - 16.0f) * 0.0f;
        canvas.DrawLine(caretX, bounds.position.y + 6.0f, caretX, bounds.position.y + bounds.size.y - 6.0f,
                        Color{255, 255, 255, 120});
    }
}
} // namespace flachead::ui
