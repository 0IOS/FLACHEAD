#pragma once

#include "../palette/Palette.hpp"

namespace flachead::theme
{
struct Spacing
{
    float base{8.0f};
    float small{4.0f};
    float large{16.0f};
    float xlarge{24.0f};
};

struct Typography
{
    float titleSize{24.0f};
    float headingSize{18.0f};
    float bodySize{16.0f};
    float captionSize{13.0f};
};

struct Radii
{
    float small{6.0f};
    float medium{12.0f};
    float large{18.0f};
    float pill{999.0f};
};

struct Motion
{
    float fast{0.12f};
    float normal{0.22f};
    float slow{0.35f};
};

// The resolved look of the shell: a derived palette plus layout and motion
// constants. A Theme is always produced from a seed colour by ThemeManager.
struct Theme
{
    flachead::palette::Palette palette;
    Spacing spacing;
    Typography typography;
    Radii radii;
    Motion motion;
};
} // namespace flachead::theme
