#include "ThemeManager.hpp"

namespace flachead::theme
{
void ThemeManager::Load(std::string_view name)
{
    m_CurrentTheme.name = std::string{name};
    m_CurrentTheme.values["accent"]     = "#7c3aed"; // violet
    m_CurrentTheme.values["accent2"]    = "#22d3ee"; // cyan
    m_CurrentTheme.values["background"] = "#05070c"; // near-black blue
    m_CurrentTheme.values["foreground"] = "#f1f5f9"; // off-white
    m_CurrentTheme.values["muted"]      = "#64748b"; // slate
    m_CurrentTheme.values["panel"]      = "#0e1118"; // dark card
    m_CurrentTheme.values["border"]     = "#1e2433"; // subtle border
    m_CurrentTheme.values["card"]       = "#141824"; // card background
    m_CurrentTheme.values["danger"]     = "#ef4444"; // red for destructive
    m_CurrentTheme.values["success"]    = "#22c55e"; // green for ok
    m_CurrentTheme.values["font"]       = "/usr/share/fonts/google-noto-vf/NotoSans[wght].ttf";
}

std::string_view ThemeManager::Get(std::string_view key) const
{
    auto it = m_CurrentTheme.values.find(key);
    if (it != m_CurrentTheme.values.end())
    {
        return it->second;
    }
    return {};
}
} // namespace flachead::theme
