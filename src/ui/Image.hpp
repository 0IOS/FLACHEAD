#pragma once

#include "Widget.hpp"

#include "../graphics/Sprite.hpp"
#include "../math/Color.hpp"

#include <memory>

namespace flachead::ui
{
// Draws an album-art tile. When no sprite texture is available it renders a
// palette-colored placeholder derived from the album name.
class Image : public Widget
{
public:
    explicit Image(std::shared_ptr<flachead::graphics::Sprite> sprite = nullptr);

    void SetSprite(std::shared_ptr<flachead::graphics::Sprite> sprite);
    std::shared_ptr<flachead::graphics::Sprite> SpriteValue() const { return m_Sprite; }

    void SetSeed(std::string seed);
    void SetPlaceholderColor(const Color& color) { m_Placeholder = color; }
    void SetCornerRadius(float radius) { m_Radius = radius; }

    Vec2 PreferredSize() const override;

protected:
    void OnDraw(Canvas& canvas) override;

private:
    std::shared_ptr<flachead::graphics::Sprite> m_Sprite;
    std::string m_Seed;
    Color m_Placeholder{60, 50, 90, 255};
    float m_Radius{14.0f};
};
} // namespace flachead::ui
