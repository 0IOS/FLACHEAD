#pragma once

#include <cstdint>

struct Color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    constexpr Color(
        uint8_t red,
        uint8_t green,
        uint8_t blue,
        uint8_t alpha = 255)
        : r(red),
          g(green),
          b(blue),
          a(alpha)
    {
    }

    static const Color White;
    static const Color Black;
    static const Color Gray;
};