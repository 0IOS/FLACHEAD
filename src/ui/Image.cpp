#include "Image.hpp"

#include "Canvas.hpp"

#include <SDL3/SDL.h>

namespace flachead::ui
{
Image::Image() = default;

void Image::SetTexture(std::shared_ptr<SDL_Texture> texture)
{
    m_Texture = std::move(texture);
}

void Image::SetSeed(std::string seed)
{
    m_Seed = std::move(seed);
}

Vec2 Image::PreferredSize() const
{
    return Vec2{120.0f, 120.0f};
}

void Image::OnDraw(Canvas& canvas)
{
    const Rect bounds = Bounds();
    if (m_Texture)
    {
        canvas.GetRenderer().DrawTexture(bounds, m_Texture.get());
        canvas.DrawRoundedRect(Rect{bounds.position.x + 0.5f, bounds.position.y + 0.5f,
                                    bounds.size.x - 1.0f, bounds.size.y - 1.0f},
                               m_Radius, Color{255, 255, 255, 26});
        return;
    }
    canvas.FillRoundedRect(bounds, m_Radius, m_Placeholder);
    canvas.DrawRoundedRect(Rect{bounds.position.x + 1.0f, bounds.position.y + 1.0f,
                                bounds.size.x - 2.0f, bounds.size.y - 2.0f},
                           m_Radius, Color{255, 255, 255, 26});
}
} // namespace flachead::ui
