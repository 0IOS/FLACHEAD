#include "Canvas.hpp"

namespace flachead::ui
{
Canvas::Canvas(flachead::core::Renderer& renderer)
    : m_Renderer(renderer)
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
} // namespace flachead::ui