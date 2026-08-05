#include "BoxLayout.hpp"

#include <algorithm>
#include <cmath>

namespace flachead::layout
{
namespace
{
float PosForAlign(AxisAlign align, float freeSpace)
{
    switch (align)
    {
        case AxisAlign::Start: return 0.0f;
        case AxisAlign::Center: return freeSpace * 0.5f;
        case AxisAlign::End: return freeSpace;
    }
    return 0.0f;
}

float CrossOffsetForAlign(CrossAlign align, float freeSpace)
{
    switch (align)
    {
        case CrossAlign::Stretch:
        case CrossAlign::Start: return 0.0f;
        case CrossAlign::Center: return freeSpace * 0.5f;
        case CrossAlign::End: return freeSpace;
    }
    return 0.0f;
}
} // namespace

Vec2 BoxLayout::Measure(const std::vector<Vec2>& preferred) const
{
    const bool horizontal = m_Orientation == Orientation::Horizontal;
    float main = 0.0f;
    float cross = 0.0f;
    for (const auto& size : preferred)
    {
        const float m = horizontal ? size.x : size.y;
        const float c = horizontal ? size.y : size.x;
        main += std::max(m, 0.0f);
        cross = std::max(cross, std::max(c, 0.0f));
    }
    const float spacing = preferred.empty() ? 0.0f : m_Spacing * static_cast<float>(preferred.size() - 1);
    main += spacing;
    main += horizontal ? (m_Insets.left + m_Insets.right) : (m_Insets.top + m_Insets.bottom);
    cross += horizontal ? (m_Insets.top + m_Insets.bottom) : (m_Insets.left + m_Insets.right);
    return horizontal ? Vec2{main, cross} : Vec2{cross, main};
}

LayoutResult BoxLayout::Layout(const std::vector<Vec2>& preferred, const Vec2& bounds) const
{
    LayoutResult result;
    result.rects.resize(preferred.size());

    const bool horizontal = m_Orientation == Orientation::Horizontal;
    const float availMain = std::max(0.0f,
        (horizontal ? bounds.x : bounds.y) - (horizontal ? (m_Insets.left + m_Insets.right)
                                                         : (m_Insets.top + m_Insets.bottom)));
    const float availCross = std::max(0.0f,
        (horizontal ? bounds.y : bounds.x) - (horizontal ? (m_Insets.top + m_Insets.bottom)
                                                         : (m_Insets.left + m_Insets.right)));

    int stretchCount = 0;
    float fixedMain = 0.0f;
    for (const auto& size : preferred)
    {
        const float m = horizontal ? size.x : size.y;
        if (m <= 0.0f)
        {
            ++stretchCount;
        }
        else
        {
            fixedMain += m;
        }
    }
    const float spacing = preferred.empty() ? 0.0f : m_Spacing * static_cast<float>(preferred.size() - 1);
    const float stretchBudget = std::max(0.0f, availMain - fixedMain - spacing);
    const float stretchSize = stretchCount > 0 ? stretchBudget / static_cast<float>(stretchCount) : 0.0f;

    float mainCursor = PosForAlign(m_Main, std::max(0.0f, availMain - fixedMain - spacing -
                                                          stretchSize * static_cast<float>(stretchCount)));
    const float mainOrigin = horizontal ? m_Insets.left : m_Insets.top;
    const float crossOrigin = horizontal ? m_Insets.top : m_Insets.left;

    for (std::size_t i = 0; i < preferred.size(); ++i)
    {
        const auto& size = preferred[i];
        float mainSize = horizontal ? size.x : size.y;
        float crossSize = horizontal ? size.y : size.x;
        if (mainSize <= 0.0f)
        {
            mainSize = stretchSize;
        }
        if (m_Cross == CrossAlign::Stretch)
        {
            crossSize = availCross;
        }
        crossSize = std::clamp(crossSize, 0.0f, availCross);

        const float crossOffset = CrossOffsetForAlign(m_Cross, availCross - crossSize);

        if (horizontal)
        {
            result.rects[i] = Rect{mainOrigin + mainCursor,
                                   crossOrigin + crossOffset,
                                   mainSize, crossSize};
        }
        else
        {
            result.rects[i] = Rect{crossOrigin + crossOffset,
                                   mainOrigin + mainCursor,
                                   crossSize, mainSize};
        }
        mainCursor += mainSize + m_Spacing;
    }

    result.contentSize = Vec2{availMain, availCross};
    return result;
}
} // namespace flachead::layout
