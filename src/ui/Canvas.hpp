#pragma once

#include "../core/Renderer.hpp"
#include "../graphics/FontManager.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"
#include "../ui/theme/ThemeManager.hpp"

namespace flachead::ui
{
class Canvas
{
public:
    Canvas(flachead::core::Renderer& renderer,
           flachead::graphics::FontManager& fontManager,
           flachead::theme::ThemeManager& themeManager);

    void FillRect(const Rect& rect, const Color& color);
    void DrawRect(const Rect& rect, const Color& color);
    void DrawText(const Rect& rect, std::string_view text, const Color& color, float fontSize = 18.0f);
    Color ThemeColor(std::string_view key, const Color& fallback) const;

private:
    flachead::core::Renderer& m_Renderer;
    flachead::graphics::FontManager& m_FontManager;
    flachead::theme::ThemeManager& m_ThemeManager;
};
} // namespace flachead::ui