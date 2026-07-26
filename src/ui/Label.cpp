#include "Label.hpp"

namespace flachead::ui
{
void Label::SetText(std::string_view text)
{
    m_Text = text;
}

std::string_view Label::Text() const
{
    return m_Text;
}
} // namespace flachead::ui
