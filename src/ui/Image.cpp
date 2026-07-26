#include "Image.hpp"

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
} // namespace flachead::ui
