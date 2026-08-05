#include "GridLayout.hpp"

#include <algorithm>

namespace flachead::layout
{
Vec2 GridLayout::Measure(const std::vector<Vec2>& preferred) const
{
    if (preferred.empty())
    {
        return {};
    }
    const int columns = std::max(1, m_Columns);
    const int rows = (static_cast<int>(preferred.size()) + columns - 1) / columns;
    float cellW = 0.0f;
    float cellH = 0.0f;
    for (const auto& size : preferred)
    {
        cellW = std::max(cellW, size.x);
        cellH = std::max(cellH, size.y);
    }
    return Vec2{cellW * columns + m_Spacing * (columns - 1),
                cellH * rows + m_Spacing * (rows - 1)};
}

LayoutResult GridLayout::Layout(const std::vector<Vec2>& preferred, const Vec2& bounds) const
{
    LayoutResult result;
    result.rects.resize(preferred.size());
    if (preferred.empty())
    {
        return result;
    }

    const int columns = std::max(1, m_Columns);
    const float cellW = std::max(0.0f,
        (bounds.x - m_Spacing * static_cast<float>(columns - 1)) / static_cast<float>(columns));

    float maxCellH = 0.0f;
    for (const auto& size : preferred)
    {
        maxCellH = std::max(maxCellH, size.y);
    }

    const int totalRows = (static_cast<int>(preferred.size()) + columns - 1) / columns;
    const float totalH = maxCellH * totalRows + m_Spacing * static_cast<float>(totalRows - 1);
    float yOffset = std::max(0.0f, (bounds.y - totalH) * 0.5f);

    for (std::size_t i = 0; i < preferred.size(); ++i)
    {
        const int col = static_cast<int>(i) % columns;
        const int row = static_cast<int>(i) / columns;
        result.rects[i] = Rect{col * (cellW + m_Spacing),
                               yOffset + row * (maxCellH + m_Spacing),
                               cellW, preferred[i].y};
    }

    result.contentSize = Vec2{bounds.x, totalH};
    return result;
}
} // namespace flachead::layout
