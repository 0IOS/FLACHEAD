#pragma once

#include "LayoutEngine.hpp"

#include <algorithm>

namespace flachead::layout
{
// Distributes children in a fixed-column grid, filling row by row. Spacing is
// applied both along the row and between rows.
class GridLayout : public LayoutEngine
{
public:
    GridLayout() = default;
    GridLayout& Columns(int columns) { m_Columns = std::max(1, columns); return *this; }
    GridLayout& Spacing(float spacing) { m_Spacing = spacing; return *this; }

    LayoutResult Layout(const std::vector<Vec2>& preferred, const Vec2& bounds) const override;
    Vec2 Measure(const std::vector<Vec2>& preferred) const override;

    int ColumnCount() const { return m_Columns; }

private:
    int m_Columns{1};
    float m_Spacing{0.0f};
};
} // namespace flachead::layout
