#pragma once

#include "Widget.hpp"

#include "../math/Color.hpp"

#include <memory>
#include <string>

struct SDL_Texture;

namespace flachead::ui
{
// Draws an album-art tile. When no texture is set it renders a palette-colored
// placeholder derived from the album name. Textures come from
// Renderer::LoadTexture and are owned by the caller.
class Image : public Widget
{
public:
    Image();

    void SetTexture(std::shared_ptr<SDL_Texture> texture);
    std::shared_ptr<SDL_Texture> TextureValue() const { return m_Texture; }

    void SetSeed(std::string seed);
    void SetPlaceholderColor(const Color& color) { m_Placeholder = color; }
    void SetCornerRadius(float radius) { m_Radius = radius; }

    Vec2 PreferredSize() const override;

protected:
    void OnDraw(Canvas& canvas) override;

private:
    std::shared_ptr<SDL_Texture> m_Texture;
    std::string m_Seed;
    Color m_Placeholder{60, 50, 90, 255};
    float m_Radius{14.0f};
};
} // namespace flachead::ui
