#pragma once

#include "Layout.hpp"
#include "../math/Rect.hpp"

namespace flachead::layout
{
class RowLayout : public Layout
{
public:
    explicit RowLayout(float spacing = 0.0f);

    void Measure(const Rect& bounds) override;
    void Arrange(const Rect& bounds) override;

private:
    float m_Spacing{0.0f};
};
} // namespace flachead::layout
