#include "Button.hpp"

#include "Canvas.hpp"
#include "../math/Color.hpp"

namespace flachead::ui
{
void Button::SetText(std::string_view text)
{
    m_Text = text;
}

std::string_view Button::Text() const
{
    return m_Text;
}

void Button::SetClickHandler(ClickHandler handler)
{
    m_ClickHandler = std::move(handler);
}

void Button::Click()
{
    if (m_ClickHandler)
    {
        m_ClickHandler();
    }
}

void Button::Draw(Canvas& canvas)
{
    if (!Visible())
    {
        return;
    }

    canvas.FillRect(Bounds(), Color::White);
    canvas.DrawRect(Bounds(), Color::Black);
}
} // namespace flachead::ui
