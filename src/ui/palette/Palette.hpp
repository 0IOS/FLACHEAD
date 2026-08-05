#pragma once

#include "../../math/Color.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace flachead::palette
{
struct Hsl
{
    float h{0.0f};
    float s{0.0f};
    float l{0.0f};
};

// Color math helpers shared across the UI shell.
namespace ColorUtil
{
Hsl ToHsl(const Color& color);
Color FromHsl(const Hsl& hsl);
Color Lighten(const Color& color, float amount);
Color Darken(const Color& color, float amount);
Color Mix(const Color& a, const Color& b, float t);
Color WithAlpha(const Color& color, uint8_t alpha);
float Luminance(const Color& color);
float ContrastRatio(const Color& a, const Color& b);
} // namespace ColorUtil

// Parses "#rrggbb" (optionally "#rrggbbaa"). Returns Black on malformed input.
Color ParseHex(std::string_view value);

// A fully derived UI palette. All surfaces and text colors are derived from a
// single accent seed, so a wallpaper or album art colour can re-theme the
// whole shell consistently.
struct Palette
{
    Color accent{124, 58, 237};
    Color accentSecondary{34, 211, 238};
    Color background{5, 7, 12};
    Color surface{14, 17, 24};
    Color surfaceRaised{20, 24, 36};
    Color foreground{241, 245, 249};
    Color muted{100, 116, 139};
    Color border{30, 36, 51};
    Color danger{239, 68, 68};
    Color success{34, 197, 94};
    Color warning{245, 158, 11};
    Color info{56, 189, 248};

    static Palette FromSeed(const Color& accent, bool dark = true);
    Palette WithAccent(const Color& accent) const;
};

// Extracts the dominant colours from an RGBA pixel buffer (used for
// album-art-driven theming). Returns candidates sorted by population,
// strongest first.
std::vector<Color> ExtractDominantColors(const uint8_t* pixels, int width, int height, int maxColors = 4);
} // namespace flachead::palette
