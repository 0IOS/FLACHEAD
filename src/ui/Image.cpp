#include "Image.hpp"

#include "Canvas.hpp"
#include "../math/Color.hpp"

namespace flachead::ui
{
Image::Image(std::shared_ptr<flachead::graphics::Sprite> sprite)
    : m_Sprite(std::move(sprite))
{
}

void Image::SetSprite(std::shared_ptr<flachead::graphics::Sprite> sprite)
{
    m_Sprite = std::move(sprite);
}

void Image::Draw(Canvas& canvas)
{
    if (!Visible())
    {
        return;
    }

    canvas.FillRect(Bounds(), Color::Gray);
    canvas.DrawRect(Bounds(), Color::White);
}
} // namespace flachead::ui
