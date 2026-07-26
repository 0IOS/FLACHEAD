#pragma once

#include "../core/Renderer.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"

namespace flachead::ui
{
class Canvas
{
public:
    explicit Canvas(flachead::core::Renderer& renderer);

    void FillRect(const Rect& rect, const Color& color);
    void DrawRect(const Rect& rect, const Color& color);

private:
    flachead::core::Renderer& m_Renderer;
};
} // namespace flachead::ui