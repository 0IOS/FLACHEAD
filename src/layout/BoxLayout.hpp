#pragma once

#include "LayoutEngine.hpp"

namespace flachead::layout
{
enum class AxisAlign
{
    Start,
    Center,
    End,
};

enum class CrossAlign
{
    Stretch,
    Start,
    Center,
    End,
};

struct Insets
{
    float left{0.0f};
    float right{0.0f};
    float top{0.0f};
    float bottom{0.0f};

    static Insets All(float value) { return {value, value, value, value}; }
};

// Result of a layout pass: one rect per child in child order, plus the size
// of the content region (the box minus padding).
// A box layout distributes children along a single axis with spacing and
// padding, with configurable main/cross axis alignment. Children that have
// preferred size 0 along the main axis grow to fill the remaining space.
class BoxLayout : public LayoutEngine
{
public:
    explicit BoxLayout(Orientation orientation)
        : m_Orientation(orientation)
    {
    }

    BoxLayout& Spacing(float spacing) { m_Spacing = spacing; return *this; }
    BoxLayout& PaddingAll(float value) { m_Insets = Insets::All(value); return *this; }
    BoxLayout& Padding(float left, float right, float top, float bottom)
    {
        m_Insets = Insets{left, right, top, bottom};
        return *this;
    }
    BoxLayout& CornerRadius(float r) { (void)r; return *this; }
    BoxLayout& MainAxis(AxisAlign align) { m_Main = align; return *this; }
    BoxLayout& CrossAxis(CrossAlign align) { m_Cross = align; return *this; }

    // Bounds is the full area the box has been given. preferred[i] is the
    // preferred size of child i (0 in the main axis means "stretch").
    LayoutResult Layout(const std::vector<Vec2>& preferred, const Vec2& bounds) const override;

    // Natural size: sum of preferred sizes + spacing + padding.
    Vec2 Measure(const std::vector<Vec2>& preferred) const override;

    Orientation OrientationValue() const { return m_Orientation; }

private:
    Orientation m_Orientation;
    float m_Spacing{0.0f};
    Insets m_Insets{};
    AxisAlign m_Main{AxisAlign::Start};
    CrossAlign m_Cross{CrossAlign::Stretch};
};
} // namespace flachead::layout
