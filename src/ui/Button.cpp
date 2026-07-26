#include "Button.hpp"

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
} // namespace flachead::ui
