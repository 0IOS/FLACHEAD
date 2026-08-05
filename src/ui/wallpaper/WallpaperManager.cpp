#include "WallpaperManager.hpp"

#include "../../core/Renderer.hpp"
#include "../Canvas.hpp"
#include "../palette/Palette.hpp"

#include <algorithm>
#include <cmath>

namespace flachead::ui::wallpaper
{
WallpaperManager::WallpaperManager(flachead::core::Renderer& renderer)
    : m_Renderer(renderer)
{
}

void WallpaperManager::SetTexture(std::shared_ptr<SDL_Texture> texture, int textureWidth, int textureHeight)
{
    if (texture == m_Texture)
    {
        return;
    }
    if (m_Texture && m_Texture != texture)
    {
        m_PreviousTexture = m_Texture;
    }
    m_Texture = std::move(texture);
    m_TextureWidth = textureWidth;
    m_TextureHeight = textureHeight;
    m_Crossfade = 0.0f;
    m_NeedsRender = true;
}

void WallpaperManager::ClearTexture()
{
    if (!m_Texture)
    {
        return;
    }
    m_PreviousTexture = m_Texture;
    m_Texture.reset();
    m_TextureWidth = 0;
    m_TextureHeight = 0;
    m_Crossfade = 0.0f;
    m_NeedsRender = true;
}

void WallpaperManager::DrawGradient(Canvas& canvas, int width, int height)
{
    const int bands = 12;
    for (int i = 0; i < bands; ++i)
    {
        const float t0 = static_cast<float>(i) / static_cast<float>(bands);
        const float t1 = static_cast<float>(i + 1) / static_cast<float>(bands);
        const float mid = (t0 + t1) * 0.5f;
        const float y = static_cast<float>(height) * t0;
        const float h = static_cast<float>(height) * (t1 - t0) + 1.0f;
        canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), h},
                        flachead::palette::ColorUtil::Mix(m_FallbackTop, m_FallbackBottom, mid));
    }
}

void WallpaperManager::DrawTextureScaled(Canvas& canvas, int width, int height,
                                         SDL_Texture* texture, float alpha)
{
    if (!texture || alpha <= 0.01f)
    {
        return;
    }

    const float textureAspect = m_TextureWidth > 0
        ? static_cast<float>(m_TextureWidth) / static_cast<float>(m_TextureHeight)
        : 1.0f;
    const float viewAspect = static_cast<float>(width) / static_cast<float>(height);

    float srcW = static_cast<float>(m_TextureWidth);
    float srcH = static_cast<float>(m_TextureHeight);
    if (textureAspect > viewAspect)
    {
        srcW = srcH * viewAspect;
    }
    else
    {
        srcH = srcW / viewAspect;
    }

    SDL_FRect src{
        (static_cast<float>(m_TextureWidth) - srcW) * 0.5f,
        (static_cast<float>(m_TextureHeight) - srcH) * 0.5f,
        srcW,
        srcH,
    };
    SDL_FRect dst{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)};

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, static_cast<uint8_t>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f));
    SDL_RenderTexture(m_Renderer.GetSDLRenderer(), texture, &src, &dst);
}

void WallpaperManager::DrawOverlays(Canvas& canvas, int width, int height)
{
    if (m_Dim > 0.0f)
    {
        canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)},
                        flachead::palette::ColorUtil::WithAlpha(Color{0, 0, 0},
                                                                static_cast<uint8_t>(m_Dim * 255.0f)));
    }
    if (m_TintStrength > 0.0f && m_Tint.a > 0)
    {
        canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)},
                        flachead::palette::ColorUtil::WithAlpha(
                            m_Tint, static_cast<uint8_t>(m_TintStrength * 255.0f)));
    }
}

void WallpaperManager::Draw(Canvas& canvas, int width, int height)
{
    const bool fadingOut = m_PreviousTexture != nullptr && m_Crossfade < 1.0f;
    if (!m_Texture || m_Crossfade < 1.0f)
    {
        DrawGradient(canvas, width, height);
    }
    if (m_Texture)
    {
        DrawTextureScaled(canvas, width, height, m_Texture.get(), m_Crossfade);
    }
    if (fadingOut)
    {
        DrawTextureScaled(canvas, width, height, m_PreviousTexture.get(), 1.0f - m_Crossfade);
    }
    DrawOverlays(canvas, width, height);

    if (m_Crossfade >= 1.0f && m_PreviousTexture)
    {
        m_PreviousTexture.reset();
    }
    m_NeedsRender = false;
}
} // namespace flachead::ui::wallpaper
