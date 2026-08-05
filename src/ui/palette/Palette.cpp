#include "Palette.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <map>

namespace flachead::palette
{
namespace
{
float Clamp01(float value)
{
    return std::max(0.0f, std::min(1.0f, value));
}

float ToFloat(uint8_t value)
{
    return static_cast<float>(value) / 255.0f;
}

uint8_t ToByte(float value)
{
    return static_cast<uint8_t>(std::round(Clamp01(value) * 255.0f));
}

float HueToRgb(float p, float q, float t)
{
    if (t < 0.0f)
    {
        t += 1.0f;
    }
    if (t > 1.0f)
    {
        t -= 1.0f;
    }
    if (t < 1.0f / 6.0f)
    {
        return p + (q - p) * 6.0f * t;
    }
    if (t < 1.0f / 2.0f)
    {
        return q;
    }
    if (t < 2.0f / 3.0f)
    {
        return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    }
    return p;
}
} // namespace

Hsl ColorUtil::ToHsl(const Color& color)
{
    const float r = ToFloat(color.r);
    const float g = ToFloat(color.g);
    const float b = ToFloat(color.b);

    const float max = std::max(r, std::max(g, b));
    const float min = std::min(r, std::min(g, b));
    const float delta = max - min;

    Hsl hsl;
    hsl.l = (max + min) * 0.5f;

    if (delta < 1e-6f)
    {
        hsl.h = 0.0f;
        hsl.s = 0.0f;
        return hsl;
    }

    hsl.s = hsl.l > 0.5f ? delta / (2.0f - max - min) : delta / (max + min);

    if (max == r)
    {
        hsl.h = (g - b) / delta + (g < b ? 6.0f : 0.0f);
    }
    else if (max == g)
    {
        hsl.h = (b - r) / delta + 2.0f;
    }
    else
    {
        hsl.h = (r - g) / delta + 4.0f;
    }
    hsl.h *= 60.0f;
    return hsl;
}

Color ColorUtil::FromHsl(const Hsl& hsl)
{
    float hue = hsl.h;
    while (hue < 0.0f)
    {
        hue += 360.0f;
    }
    while (hue >= 360.0f)
    {
        hue -= 360.0f;
    }
    const float h = hue / 360.0f;
    const float s = Clamp01(hsl.s);
    const float l = Clamp01(hsl.l);

    if (s < 1e-6f)
    {
        const uint8_t v = ToByte(l);
        return Color{v, v, v, 255};
    }

    const float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
    const float p = 2.0f * l - q;
    return Color{
        ToByte(HueToRgb(p, q, h + 1.0f / 3.0f)),
        ToByte(HueToRgb(p, q, h)),
        ToByte(HueToRgb(p, q, h - 1.0f / 3.0f)),
        255};
}

Color ColorUtil::Lighten(const Color& color, float amount)
{
    const Hsl hsl = ToHsl(color);
    return FromHsl(Hsl{hsl.h, hsl.s, Clamp01(hsl.l + amount)});
}

Color ColorUtil::Darken(const Color& color, float amount)
{
    return Lighten(color, -amount);
}

Color ColorUtil::Mix(const Color& a, const Color& b, float t)
{
    t = Clamp01(t);
    return Color{
        ToByte(ToFloat(a.r) + (ToFloat(b.r) - ToFloat(a.r)) * t),
        ToByte(ToFloat(a.g) + (ToFloat(b.g) - ToFloat(a.g)) * t),
        ToByte(ToFloat(a.b) + (ToFloat(b.b) - ToFloat(a.b)) * t),
        static_cast<uint8_t>(std::round(static_cast<float>(a.a) +
                                        (static_cast<float>(b.a) - static_cast<float>(a.a)) * t))};
}

Color ColorUtil::WithAlpha(const Color& color, uint8_t alpha)
{
    return Color{color.r, color.g, color.b, alpha};
}

float ColorUtil::Luminance(const Color& color)
{
    const auto channel = [](uint8_t value) {
        const float c = ToFloat(value);
        return c <= 0.03928f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
    };
    return 0.2126f * channel(color.r) + 0.7152f * channel(color.g) + 0.0722f * channel(color.b);
}

float ColorUtil::ContrastRatio(const Color& a, const Color& b)
{
    const float l1 = Luminance(a);
    const float l2 = Luminance(b);
    const float lighter = std::max(l1, l2);
    const float darker = std::min(l1, l2);
    return (lighter + 0.05f) / (darker + 0.05f);
}

Color ParseHex(std::string_view value)
{
    const auto byte = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9')
        {
            return static_cast<uint8_t>(c - '0');
        }
        if (c >= 'a' && c <= 'f')
        {
            return static_cast<uint8_t>(10 + c - 'a');
        }
        if (c >= 'A' && c <= 'F')
        {
            return static_cast<uint8_t>(10 + c - 'A');
        }
        return 0;
    };

    const bool hasAlpha = value.size() == 9;
    if (value.empty() || value[0] != '#' || (value.size() != 7 && !hasAlpha))
    {
        return Color::Black;
    }
    return Color{
        static_cast<uint8_t>((byte(value[1]) << 4) | byte(value[2])),
        static_cast<uint8_t>((byte(value[3]) << 4) | byte(value[4])),
        static_cast<uint8_t>((byte(value[5]) << 4) | byte(value[6])),
        hasAlpha ? static_cast<uint8_t>((byte(value[7]) << 4) | byte(value[8])) : 255};
}

Palette Palette::FromSeed(const Color& accent, bool dark)
{
    Palette p;
    p.accent = accent;

    const Hsl seed = ColorUtil::ToHsl(accent);
    const float seedHue = seed.h;
    const float seedSat = std::max(0.45f, seed.s);

    p.accentSecondary = ColorUtil::FromHsl(Hsl{
        std::fmod(seedHue + 42.0f, 360.0f),
        seedSat,
        dark ? 0.72f : 0.45f});

    if (dark)
    {
        const float bgL = 0.035f;
        const float hueTint = 0.06f;
        const Color tinted{ColorUtil::FromHsl(Hsl{seedHue, seedSat * hueTint, bgL})};
        p.background = tinted;
        p.surface = ColorUtil::Mix(tinted, Color{255, 255, 255}, 0.045f);
        p.surfaceRaised = ColorUtil::Mix(tinted, Color{255, 255, 255}, 0.085f);
        p.foreground = Color{241, 245, 249};
        p.muted = ColorUtil::Mix(tinted, Color{255, 255, 255}, 0.32f);
        p.border = ColorUtil::Mix(tinted, Color{255, 255, 255}, 0.14f);
    }
    else
    {
        const float bgL = 0.97f;
        const Color tinted{ColorUtil::FromHsl(Hsl{seedHue, seedSat * 0.06f, bgL})};
        p.background = tinted;
        p.surface = ColorUtil::Mix(tinted, Color{255, 255, 255}, 0.35f);
        p.surfaceRaised = Color::White;
        p.foreground = Color{15, 23, 42};
        p.muted = Color{100, 116, 139};
        p.border = ColorUtil::Mix(tinted, Color{15, 23, 42}, 0.12f);
    }

    p.danger = Color{239, 68, 68};
    p.success = Color{34, 197, 94};
    p.warning = Color{245, 158, 11};
    p.info = Color{56, 189, 248};
    return p;
}

Palette Palette::WithAccent(const Color& accent) const
{
    return FromSeed(accent, ColorUtil::Luminance(background) < 0.3f);
}

namespace
{
struct Hsv
{
    float h{0.0f};
    float s{0.0f};
    float v{0.0f};
};

Hsv ToHsv(const Color& color)
{
    const float r = ToFloat(color.r);
    const float g = ToFloat(color.g);
    const float b = ToFloat(color.b);
    const float max = std::max(r, std::max(g, b));
    const float min = std::min(r, std::min(g, b));
    const float delta = max - min;

    Hsv hsv;
    hsv.v = max;
    hsv.s = max > 0.0f ? delta / max : 0.0f;

    if (delta < 1e-6f)
    {
        hsv.h = 0.0f;
    }
    else if (max == r)
    {
        hsv.h = 60.0f * std::fmod((g - b) / delta, 6.0f);
    }
    else if (max == g)
    {
        hsv.h = 60.0f * ((b - r) / delta + 2.0f);
    }
    else
    {
        hsv.h = 60.0f * ((r - g) / delta + 4.0f);
    }
    if (hsv.h < 0.0f)
    {
        hsv.h += 360.0f;
    }
    return hsv;
}
} // namespace

std::vector<Color> ExtractDominantColors(const uint8_t* pixels, int width, int height, int maxColors)
{
    std::vector<Color> result;
    if (!pixels || width <= 0 || height <= 0 || maxColors <= 0)
    {
        return result;
    }

    const int step = std::max(1, (width * height) / 4000);
    const int hueBins = 18;
    const float satMin = 0.18f;
    const float valMin = 0.12f;

    std::array<int, hueBins> counts{};
    std::array<float, hueBins> sumH{};
    std::array<float, hueBins> sumS{};
    std::array<float, hueBins> sumV{};

    const int totalPixels = width * height;
    for (int i = 0; i < totalPixels; i += step)
    {
        const int offset = i * 4;
        const Color pixel{pixels[offset], pixels[offset + 1], pixels[offset + 2], pixels[offset + 3]};
        const Hsv hsv = ToHsv(pixel);
        if (hsv.s < satMin || hsv.v < valMin)
        {
            continue;
        }
        int bin = static_cast<int>(hsv.h / 360.0f * hueBins);
        bin = std::clamp(bin, 0, hueBins - 1);
        ++counts[bin];
        sumH[bin] += hsv.h;
        sumS[bin] += hsv.s;
        sumV[bin] += hsv.v;
    }

    std::vector<std::pair<int, int>> ranked;
    for (int bin = 0; bin < hueBins; ++bin)
    {
        if (counts[bin] > 0)
        {
            ranked.emplace_back(counts[bin], bin);
        }
    }
    std::sort(ranked.begin(), ranked.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    for (std::size_t i = 0; i < ranked.size() && result.size() < static_cast<std::size_t>(maxColors); ++i)
    {
        const int bin = ranked[i].second;
        const float h = sumH[bin] / static_cast<float>(counts[bin]);
        const float s = sumS[bin] / static_cast<float>(counts[bin]);
        const float v = std::min(0.85f, sumV[bin] / static_cast<float>(counts[bin]));
        result.push_back(ColorUtil::FromHsl(Hsl{h, s, v * 0.5f + 0.25f}));
    }
    return result;
}
} // namespace flachead::palette
