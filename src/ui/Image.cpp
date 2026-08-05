#include "Image.hpp"

#include "Canvas.hpp"

#include <utility>

namespace flachead::ui
{
Image::Image(std::shared_ptr<flachead::graphics::Sprite> sprite)
    : m_Sprite(std::move(sprite))
{
    SetFocusable(false);
}

void Image::SetSprite(std::shared_ptr<flachead::graphics::Sprite> sprite)
{
    m_Sprite = std::move(sprite);
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
    canvas.FillRoundedRect(bounds, m_Radius, m_Placeholder);
    canvas.DrawRoundedRect(Rect{bounds.position.x + 1.0f, bounds.position.y + 1.0f,
                                bounds.size.x - 2.0f, bounds.size.y - 2.0f},
                           m_Radius, Color{255, 255, 255, 26});
}
} // namespace flachead::ui
