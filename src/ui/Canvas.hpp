#pragma once

#include "../core/Renderer.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"

class Canvas
{
public:
    explicit Canvas(Renderer& renderer);

    void FillRect(const Rect& rect, const Color& color);
    void DrawRect(const Rect& rect, const Color& color);

private:
    Renderer& m_Renderer;
};