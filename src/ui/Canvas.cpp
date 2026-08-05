#include "Canvas.hpp"

#include <SDL3_ttf/SDL_ttf.h>

#include <algorithm>
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
                return static_cast<uint8_t>(c - '0');
            if (c >= 'a' && c <= 'f')
                return static_cast<uint8_t>(10 + c - 'a');
            if (c >= 'A' && c <= 'F')
                return static_cast<uint8_t>(10 + c - 'A');
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

constexpr std::string_view kFallbackFont = "/usr/share/fonts/google-noto-vf/NotoSans[wght].ttf";
constexpr std::string_view kSystemFont   = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
} // namespace

Canvas::Canvas(flachead::core::Renderer& renderer,
               flachead::graphics::FontManager& fontManager,
               flachead::theme::ThemeManager& themeManager)
    : m_Renderer(renderer),
      m_FontManager(fontManager),
      m_ThemeManager(themeManager)
{
}

const std::string& Canvas::FontPath() const
{
    if (m_CachedFontPath.empty())
    {
        const std::string_view themeFont = m_ThemeManager.Get("font");
        if (!themeFont.empty())
            m_CachedFontPath = std::string{themeFont};
        else
            m_CachedFontPath = std::string{kFallbackFont};
    }
    return m_CachedFontPath;
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

void Canvas::FillRoundedRect(const Rect& rect, float radius, const Color& color)
{
    m_Renderer.SetColor(color);
    m_Renderer.FillRoundedRect(rect, radius);
}

void Canvas::DrawRoundedRect(const Rect& rect, float radius, const Color& color)
{
    m_Renderer.SetColor(color);
    m_Renderer.DrawRoundedRect(rect, radius);
}

void Canvas::DrawLine(float x1, float y1, float x2, float y2, const Color& color)
{
    m_Renderer.SetColor(color);
    m_Renderer.DrawLine(x1, y1, x2, y2);
}

void Canvas::FillCircle(float cx, float cy, float radius, const Color& color)
{
    m_Renderer.SetColor(color);
    m_Renderer.FillCircle(cx, cy, radius);
}

void Canvas::DrawCircle(float cx, float cy, float radius, const Color& color)
{
    m_Renderer.SetColor(color);
    m_Renderer.DrawCircle(cx, cy, radius);
}

void Canvas::DrawText(const Rect& rect, std::string_view text, const Color& color, float fontSize)
{
    std::shared_ptr<flachead::graphics::Font> font;
    if (const std::string& fontPath = FontPath(); !fontPath.empty())
    {
        font = m_FontManager.Acquire(fontPath, fontSize);
    }

    if (!font || !font->Valid())
    {
        font = m_FontManager.Acquire(kSystemFont, fontSize);
    }

    if (font && font->Valid())
    {
        m_Renderer.DrawText(rect, text, *font, color);
    }
}

void Canvas::DrawTextCentered(const Rect& rect, std::string_view text, const Color& color, float fontSize)
{
    if (text.empty())
    {
        return;
    }
    std::shared_ptr<flachead::graphics::Font> font;
    if (const std::string& fontPath = FontPath(); !fontPath.empty())
    {
        font = m_FontManager.Acquire(fontPath, fontSize);
    }
    if (!font || !font->Valid())
    {
        font = m_FontManager.Acquire(kSystemFont, fontSize);
    }
    if (!font || !font->Valid())
    {
        return;
    }

    int width = 0;
    int height = 0;
    if (TTF_GetStringSize(font->Native(), text.data(), text.size(), &width, &height))
    {
        const float x = rect.position.x + std::max(0.0f, (rect.size.x - static_cast<float>(width)) * 0.5f);
        const float y = rect.position.y + std::max(0.0f, (rect.size.y - static_cast<float>(height)) * 0.5f);
        m_Renderer.DrawText(Rect{x, y, static_cast<float>(width), static_cast<float>(height)},
                            text, *font, color);
        return;
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