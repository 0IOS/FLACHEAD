#include "Canvas.hpp"

#include <cctype>
#include <cstdlib>
#include <memory>
#include <string>

namespace flachead::ui
{
namespace
{
Color ParseColor(std::string_view value)
{
    if (value.size() != 7 || value[0] != '#')
    {
        return Color::White;
    }

    const auto parseByte = [](char high, char low) -> uint8_t {
        auto toValue = [](char c) -> uint8_t {
            if (c >= '0' && c <= '9')
            {
                return static_cast<uint8_t>(c - '0');
            }
            if (c >= 'a' && c <= 'f')
            {
                return static_cast<uint8_t>(10 + c - 'a');
            }
            if (c >= 'A' && c <= 'F')
            {
                return static_cast<uint8_t>(10 + c - 'A');
            }
            return 0;
        };

        return static_cast<uint8_t>((toValue(high) << 4) | toValue(low));
    };

    return Color{
        parseByte(value[1], value[2]),
        parseByte(value[3], value[4]),
        parseByte(value[5], value[6]),
        255};
}
} // namespace

Canvas::Canvas(flachead::core::Renderer& renderer,
               flachead::graphics::FontManager& fontManager,
               flachead::theme::ThemeManager& themeManager)
    : m_Renderer(renderer),
      m_FontManager(fontManager),
      m_ThemeManager(themeManager)
{
}

void Canvas::FillRect(const Rect& rect, const Color& color)
{
    m_Renderer.SetColor(color);
    m_Renderer.FillRect(rect);
}

void Canvas::DrawRect(const Rect& rect, const Color& color)
{
    m_Renderer.SetColor(color);
    m_Renderer.DrawRect(rect);
}

void Canvas::DrawText(const Rect& rect, std::string_view text, const Color& color, float fontSize)
{
    const std::string fontPath = std::string{m_ThemeManager.Get("font")};
    auto font = m_FontManager.Acquire(fontPath.empty() ? "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf" : fontPath);
    if (!font || !font->Valid())
    {
        return;
    }

    if (fontSize > 0.0f)
    {
        auto fallback = std::make_shared<flachead::graphics::Font>(fontPath.empty() ? std::string_view{"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"} : std::string_view{fontPath}, fontSize);
        if (fallback->Valid())
        {
            m_Renderer.DrawText(rect, text, *fallback, color);
            return;
        }
    }

    m_Renderer.DrawText(rect, text, *font, color);
}

Color Canvas::ThemeColor(std::string_view key, const Color& fallback) const
{
    const std::string_view themeValue = m_ThemeManager.Get(key);
    if (themeValue.empty())
    {
        return fallback;
    }

    return ParseColor(themeValue);
}
} // namespace flachead::ui