#include "ThemeManager.hpp"

namespace flachead::theme
{
void ThemeManager::Load(std::string_view name)
{
    m_CurrentTheme.name = std::string{name};
    m_CurrentTheme.values["accent"] = "#7c3aed";
    m_CurrentTheme.values["background"] = "#05070b";
    m_CurrentTheme.values["foreground"] = "#f8fafc";
    m_CurrentTheme.values["muted"] = "#94a3b8";
    m_CurrentTheme.values["panel"] = "#111827";
    m_CurrentTheme.values["font"] = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
}

std::string_view ThemeManager::Get(std::string_view key) const
{
    auto it = m_CurrentTheme.values.find(std::string{key});
    if (it != m_CurrentTheme.values.end())
    {
        return it->second;
    }
    return {};
}
} // namespace flachead::theme
