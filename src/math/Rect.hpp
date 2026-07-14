#pragma once

#include "Vec2.hpp"

struct Rect
{
    Vec2 position;
    Vec2 size;

    Rect() = default;

    Rect(float x, float y,
         float width,
         float height)
        : position(x, y),
          size(width, height)
    {
    }
};