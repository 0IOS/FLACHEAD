#pragma once

#include "Widget.hpp"

#include "../math/Color.hpp"

namespace flachead::ui
{
// A thin vertical scroll indicator. Value is the scroll offset in [0,1];
// viewport fraction controls the thumb length.
class ScrollBar : public Widget
{
public:
    ScrollBar();

    void SetValue(float value);
    float Value() const { return m_Value; }
    void SetViewportFraction(float fraction);
    float ViewportFraction() const { return m_Fraction; }

    void SetColor(const Color& color) { m_Color = color; }

    Vec2 PreferredSize() const override;

protected:
    void OnDraw(Canvas& canvas) override;

private:
    float m_Value{0.0f};
    float m_Fraction{0.8f};
    Color m_Color{90, 96, 110};
};
} // namespace flachead::ui
