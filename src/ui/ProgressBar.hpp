#pragma once

#include "Widget.hpp"

#include "../math/Color.hpp"

namespace flachead::ui
{
// A determinate progress bar. Value is clamped to [0, 1].
class ProgressBar : public Widget
{
public:
    ProgressBar();

    void SetValue(float value);
    float Value() const { return m_Value; }

    void SetColor(const Color& color) { m_Color = color; }
    void SetTrackColor(const Color& color) { m_TrackColor = color; }
    void SetCornerRadius(float radius) { m_Radius = radius; }

    Vec2 PreferredSize() const override;

protected:
    void OnDraw(Canvas& canvas) override;

private:
    float m_Value{0.0f};
    Color m_Color{124, 58, 237};
    Color m_TrackColor{30, 34, 46};
    float m_Radius{5.0f};
};
} // namespace flachead::ui
