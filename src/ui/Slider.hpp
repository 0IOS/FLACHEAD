#pragma once

#include "Widget.hpp"

#include "../math/Color.hpp"

#include <functional>

namespace flachead::ui
{
// A draggable slider over [min, max]. Reports value changes through the value
// callback; drag and tap both seek directly.
class Slider : public Widget
{
public:
    using ChangeCallback = std::function<void(float)>;

    Slider();

    void SetRange(float min, float max);
    void SetValue(float value);
    float Value() const { return m_Value; }
    float MinValue() const { return m_Min; }
    float MaxValue() const { return m_Max; }

    void SetColor(const Color& color) { m_Color = color; }
    void SetTrackColor(const Color& color) { m_TrackColor = color; }
    void SetChangeCallback(ChangeCallback callback) { m_OnChange = std::move(callback); }

    Vec2 PreferredSize() const override;

protected:
    void OnDraw(Canvas& canvas) override;
    bool OnHandleEvent(const flachead::input::InputEvent& event) override;

private:
    void SeekToPosition(float x);

    float m_Min{0.0f};
    float m_Max{1.0f};
    float m_Value{0.0f};
    Color m_Color{124, 58, 237};
    Color m_TrackColor{30, 34, 46};
    ChangeCallback m_OnChange;
    bool m_Dragging{false};
};
} // namespace flachead::ui
