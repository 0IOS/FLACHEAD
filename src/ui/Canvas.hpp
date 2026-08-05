#pragma once

#include "../core/Renderer.hpp"
#include "../graphics/FontManager.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"
#include "../ui/theme/ThemeManager.hpp"

#include <algorithm>
#include <string_view>

namespace flachead::ui
{
class Canvas
{
public:
    Canvas(flachead::core::Renderer& renderer,
           flachead::graphics::FontManager& fontManager,
           flachead::theme::ThemeManager& themeManager);

    // Basic geometry
    void FillRect(const Rect& rect, const Color& color);
    void DrawRect(const Rect& rect, const Color& color);
    void FillRoundedRect(const Rect& rect, float radius, const Color& color);
    void DrawRoundedRect(const Rect& rect, float radius, const Color& color);
    void DrawLine(float x1, float y1, float x2, float y2, const Color& color);
    void FillCircle(float cx, float cy, float radius, const Color& color);
    void DrawCircle(float cx, float cy, float radius, const Color& color);

    // Text
    void DrawText(const Rect& rect, std::string_view text, const Color& color, float fontSize = 18.0f);
    void DrawTextCentered(const Rect& rect, std::string_view text, const Color& color, float fontSize = 18.0f);

    // UI scale relative to the 240x320 reference environment. Applied to text
    // so typography grows with the window while layout stays in logical pixels.
    void SetScale(float scale) { m_Scale = std::max(1.0f, scale); }
    float Scale() const { return m_Scale; }

    // Theme
    Color ThemeColor(std::string_view key, const Color& fallback) const;

    // SDL access (for advanced drawing only)
    flachead::core::Renderer& GetRenderer() { return m_Renderer; }

private:
    const std::string& FontPath() const;

    flachead::core::Renderer& m_Renderer;
    flachead::graphics::FontManager& m_FontManager;
    flachead::theme::ThemeManager& m_ThemeManager;
    float m_Scale{1.0f};
    mutable std::string m_CachedFontPath;
};
} // namespace flachead::ui