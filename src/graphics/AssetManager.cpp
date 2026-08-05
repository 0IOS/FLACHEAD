#include "AssetManager.hpp"

#include "../core/Renderer.hpp"
#include "../ui/palette/Palette.hpp"

#include <algorithm>

namespace flachead::graphics
{
namespace
{
uint8_t Blend(uint8_t a, uint8_t b, float t)
{
    return static_cast<uint8_t>(std::clamp(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t,
                                           0.0f, 255.0f));
}
} // namespace

AssetManager::AssetManager(flachead::core::Renderer& renderer)
    : m_Renderer(renderer)
{
}

std::string AssetManager::Key(std::string_view kind, int width, int height, const Color& a, const Color& b) const
{
    std::string key{kind};
    key += ":";
    key += std::to_string(width);
    key += "x";
    key += std::to_string(height);
    const auto nib = [](uint8_t v) -> char {
        constexpr char digits[] = "0123456789abcdef";
        return digits[v & 0x0F];
    };
    const auto append = [&key, &nib](const Color& c) {
        key += nib(c.r >> 4);
        key += nib(c.r);
        key += nib(c.g >> 4);
        key += nib(c.g);
        key += nib(c.b >> 4);
        key += nib(c.b);
        key += nib(c.a >> 4);
        key += nib(c.a);
    };
    append(a);
    append(b);
    return key;
}

std::shared_ptr<SDL_Texture> AssetManager::Solid(int width, int height, const Color& color)
{
    const std::string key = Key("solid", width, height, color, color);
    if (auto it = m_Assets.find(key); it != m_Assets.end())
    {
        return it->second;
    }

    SDL_Texture* texture = SDL_CreateTexture(m_Renderer.GetSDLRenderer(), SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STATIC, width, height);
    if (!texture)
    {
        return nullptr;
    }
    std::vector<uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
    for (std::size_t i = 0; i < pixels.size(); i += 4)
    {
        pixels[i] = color.r;
        pixels[i + 1] = color.g;
        pixels[i + 2] = color.b;
        pixels[i + 3] = color.a;
    }
    SDL_UpdateTexture(texture, nullptr, pixels.data(), width * 4);
    auto asset = std::shared_ptr<SDL_Texture>(texture, [](SDL_Texture* t) { SDL_DestroyTexture(t); });
    m_Assets.emplace(key, asset);
    return asset;
}

std::shared_ptr<SDL_Texture> AssetManager::Gradient(int width, int height, const Color& top, const Color& bottom)
{
    const std::string key = Key("gradient", width, height, top, bottom);
    if (auto it = m_Assets.find(key); it != m_Assets.end())
    {
        return it->second;
    }

    SDL_Texture* texture = SDL_CreateTexture(m_Renderer.GetSDLRenderer(), SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STATIC, width, height);
    if (!texture)
    {
        return nullptr;
    }
    std::vector<uint8_t> pixels(static_cast<std::size_t>(width) * height * 4);
    for (int y = 0; y < height; ++y)
    {
        const float t = height > 1 ? static_cast<float>(y) / static_cast<float>(height - 1) : 0.0f;
        const uint8_t r = Blend(top.r, bottom.r, t);
        const uint8_t g = Blend(top.g, bottom.g, t);
        const uint8_t b = Blend(top.b, bottom.b, t);
        const uint8_t a = Blend(top.a, bottom.a, t);
        for (int x = 0; x < width; ++x)
        {
            const std::size_t offset = (static_cast<std::size_t>(y) * width + x) * 4;
            pixels[offset] = r;
            pixels[offset + 1] = g;
            pixels[offset + 2] = b;
            pixels[offset + 3] = a;
        }
    }
    SDL_UpdateTexture(texture, nullptr, pixels.data(), width * 4);
    auto asset = std::shared_ptr<SDL_Texture>(texture, [](SDL_Texture* t) { SDL_DestroyTexture(t); });
    m_Assets.emplace(key, asset);
    return asset;
}

std::shared_ptr<SDL_Texture> AssetManager::RoundedPanel(int width, int height, float radius, const Color& fill,
                                                        const Color& border, float borderWidth)
{
    const std::string key = Key("panel:" + std::to_string(static_cast<int>(radius)) + ":" +
                                    std::to_string(static_cast<int>(borderWidth)),
                                width, height, fill, border);
    if (auto it = m_Assets.find(key); it != m_Assets.end())
    {
        return it->second;
    }

    SDL_Texture* texture = SDL_CreateTexture(m_Renderer.GetSDLRenderer(), SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_STATIC, width, height);
    if (!texture)
    {
        return nullptr;
    }

    const float r = std::max(0.0f, radius);
    std::vector<uint8_t> pixels(static_cast<std::size_t>(width) * height * 4, 0);
    const auto inRound = [r](float dx, float dy) {
        return dx * dx + dy * dy <= r * r;
    };
    const auto alphaAt = [&](float x, float y) -> uint8_t {
        if (x < 0.0f || y < 0.0f || x >= static_cast<float>(width) || y >= static_cast<float>(height))
        {
            return 0;
        }
        const float nearestX = std::clamp(x, r, static_cast<float>(width) - r);
        const float nearestY = std::clamp(y, r, static_cast<float>(height) - r);
        const float dx = x - nearestX;
        const float dy = y - nearestY;
        if (dx == 0.0f || dy == 0.0f)
        {
            return 255;
        }
        return inRound(dx, dy) ? 255 : 0;
    };

    const int borderPx = static_cast<int>(std::max(0.0f, borderWidth));
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const uint8_t fillAlpha = alphaAt(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
            if (fillAlpha == 0)
            {
                continue;
            }
            const bool nearBorder = borderPx > 0 &&
                                    (x < borderPx || x >= width - borderPx ||
                                     y < borderPx || y >= height - borderPx);
            const Color& color = nearBorder ? border : fill;
            const std::size_t offset = (static_cast<std::size_t>(y) * width + x) * 4;
            pixels[offset] = color.r;
            pixels[offset + 1] = color.g;
            pixels[offset + 2] = color.b;
            pixels[offset + 3] = fillAlpha;
        }
    }
    SDL_UpdateTexture(texture, nullptr, pixels.data(), width * 4);
    auto asset = std::shared_ptr<SDL_Texture>(texture, [](SDL_Texture* t) { SDL_DestroyTexture(t); });
    m_Assets.emplace(key, asset);
    return asset;
}

void AssetManager::Clear()
{
    m_Assets.clear();
}
} // namespace flachead::graphics
