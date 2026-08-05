#pragma once

#include "../palette/Palette.hpp"
#include "Theme.hpp"

#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace flachead::theme
{
// Owns the resolved Theme and keeps a flat string view of the palette for
// code that reads colours by key. ApplySeed re-derives the whole palette
// from one accent colour; Load restores a named built-in theme.
class ThemeManager
{
public:
    ThemeManager() = default;

    void Load(std::string_view name);
    void ApplySeed(const Color& accent, bool dark = true);
    void SetFont(std::string fontPath);

    const flachead::palette::Palette& ActivePalette() const { return m_Theme.palette; }
    const Theme& CurrentTheme() const { return m_Theme; }
    Theme& CurrentThemeMutable() { return m_Theme; }

    // Flat key access, retained for legacy callers.
    std::string_view Get(std::string_view key) const;

    bool IsDark() const { return m_Dark; }

private:
    void SyncStrings();

    Theme m_Theme;
    std::map<std::string, std::string, std::less<>> m_Values;
    bool m_Dark{true};
};
} // namespace flachead::theme
