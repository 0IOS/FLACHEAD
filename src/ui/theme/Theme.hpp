#pragma once

#include <string>
#include <string_view>

namespace flachead::theme
{
struct ColorPalette
{
    std::string primary;
    std::string secondary;
    std::string background;
    std::string foreground;
};

struct Spacing
{
    float base{8.0f};
    float small{4.0f};
    float large{16.0f};
};

struct Typography
{
    float titleSize{28.0f};
    float bodySize{18.0f};
};

class Theme
{
public:
    Theme() = default;

    void SetPalette(const ColorPalette& palette);
    void SetSpacing(const Spacing& spacing);
    void SetTypography(const Typography& typography);

    const ColorPalette& Palette() const;
    const Spacing& SpacingValues() const;
    const Typography& TypographyValues() const;

private:
    ColorPalette m_Palette;
    Spacing m_Spacing;
    Typography m_Typography;
};
} // namespace flachead::theme
