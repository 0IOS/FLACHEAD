#pragma once

#include "../../math/Color.hpp"

#include <SDL3/SDL.h>

#include <memory>

namespace flachead::core
{
class Renderer;
}

namespace flachead::ui
{
class Canvas;

namespace wallpaper
{
// Renders the full-screen background behind every screen. Supports a
// crossfading image texture (album art / photos), a gradient fallback, and
// dim / tint layers so foreground surfaces stay readable. Texture lifecycle
// is owned by the caller; the manager borrows a renderer reference.
class WallpaperManager
{
public:
    explicit WallpaperManager(flachead::core::Renderer& renderer);

    void SetTexture(std::shared_ptr<SDL_Texture> texture, int textureWidth, int textureHeight);
    void ClearTexture();
    bool HasTexture() const { return m_Texture != nullptr; }

    void SetFallback(const Color& top, const Color& bottom);
    void SetDim(float dim) { m_Dim = dim; }
    void SetTint(const Color& tint, float strength) { m_Tint = tint; m_TintStrength = strength; }

    // Crossfade progress toward the texture set by SetTexture. 0 = old look,
    // 1 = new texture fully visible. Drive this from an animation.
    void SetCrossfade(float progress) { m_Crossfade = progress; }
    float Crossfade() const { return m_Crossfade; }

    void Draw(Canvas& canvas, int width, int height);

    bool NeedsRender() const { return m_NeedsRender; }
    void ClearDirty() { m_NeedsRender = false; }

private:
    void DrawGradient(Canvas& canvas, int width, int height);
    void DrawTextureScaled(Canvas& canvas, int width, int height, SDL_Texture* texture, float alpha);
    void DrawOverlays(Canvas& canvas, int width, int height);

    flachead::core::Renderer& m_Renderer;
    std::shared_ptr<SDL_Texture> m_Texture;
    std::shared_ptr<SDL_Texture> m_PreviousTexture;
    int m_TextureWidth{0};
    int m_TextureHeight{0};

    Color m_FallbackTop{5, 7, 12};
    Color m_FallbackBottom{15, 11, 28};
    float m_Dim{0.0f};
    Color m_Tint{0, 0, 0};
    float m_TintStrength{0.0f};
    float m_Crossfade{0.0f};

    bool m_NeedsRender{true};
};
} // namespace flachead::wallpaper
} // namespace flachead::ui
