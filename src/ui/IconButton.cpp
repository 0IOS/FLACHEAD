#include "IconButton.hpp"

namespace flachead::ui
{
IconButton::IconButton(std::string_view iconName)
    : m_IconName(iconName)
{
}

void IconButton::SetIcon(std::string_view iconName)
{
    m_IconName = iconName;
}

std::string_view IconButton::Icon() const
{
    return m_IconName;
}
} // namespace flachead::ui
