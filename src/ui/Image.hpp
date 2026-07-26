#pragma once

#include "Widget.hpp"
#include "../graphics/Sprite.hpp"

namespace flachead::ui
{
class Image : public Widget
{
public:
    explicit Image(std::shared_ptr<flachead::graphics::Sprite> sprite = nullptr);
    void SetSprite(std::shared_ptr<flachead::graphics::Sprite> sprite);

private:
    std::shared_ptr<flachead::graphics::Sprite> m_Sprite;
};
} // namespace flachead::ui
