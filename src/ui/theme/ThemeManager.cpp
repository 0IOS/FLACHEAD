#include "ThemeManager.hpp"

#include <charconv>

namespace flachead::theme
{
namespace
{
std::string Hex(const Color& color)
{
    const auto nib = [](uint8_t v) -> char {
        constexpr char digits[] = "0123456789abcdef";
        return digits[v & 0x0F];
    };
    std::string out = "#";
    out += nib(color.r >> 4);
    out += nib(color.r);
    out += nib(color.g >> 4);
    out += nib(color.g);
    out += nib(color.b >> 4);
    out += nib(color.b);
    return out;
}
} // namespace

void ThemeManager::SyncStrings()
{
    const auto& p = m_Theme.palette;
    m_Values["accent"]     = Hex(p.accent);
    m_Values["accent2"]    = Hex(p.accentSecondary);
    m_Values["background"] = Hex(p.background);
    m_Values["foreground"] = Hex(p.foreground);
    m_Values["muted"]      = Hex(p.muted);
    m_Values["panel"]      = Hex(p.surface);
    m_Values["card"]       = Hex(p.surfaceRaised);
    m_Values["border"]     = Hex(p.border);
    m_Values["danger"]     = Hex(p.danger);
    m_Values["success"]    = Hex(p.success);
}

void ThemeManager::Load(std::string_view name)
{
    (void)name;
    ApplySeed(Color{124, 58, 237}, true);
    m_Values["font"] = "/usr/share/fonts/google-noto-vf/NotoSans[wght].ttf";
}

void ThemeManager::ApplySeed(const Color& accent, bool dark)
{
    m_Dark = dark;
    m_Theme.palette = flachead::palette::Palette::FromSeed(accent, dark);
    SyncStrings();
}

void ThemeManager::SetFont(std::string fontPath)
{
    m_Values["font"] = std::move(fontPath);
}

std::string_view ThemeManager::Get(std::string_view key) const
{
    auto it = m_Values.find(key);
    if (it != m_Values.end())
    {
        return it->second;
    }
    return {};
}
} // namespace flachead::theme
