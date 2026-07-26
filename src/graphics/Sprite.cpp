#include "Sprite.hpp"

#include <utility>

namespace flachead::graphics
{
Sprite::Sprite(std::shared_ptr<Texture> texture)
    : m_Texture(std::move(texture))
{
}

void Sprite::SetTexture(std::shared_ptr<Texture> texture)
{
    m_Texture = std::move(texture);
}

std::shared_ptr<Texture> Sprite::GetTexture() const
{
    return m_Texture;
}
} // namespace flachead::graphics
