#pragma once

#include "Texture.hpp"

#include <memory>

namespace flachead::graphics
{
class Sprite
{
public:
    explicit Sprite(std::shared_ptr<Texture> texture = nullptr);
    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const;

private:
    std::shared_ptr<Texture> m_Texture;
};
} // namespace flachead::graphics
