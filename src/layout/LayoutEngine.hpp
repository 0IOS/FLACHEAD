#pragma once

#include "../math/Rect.hpp"

#include <memory>
#include <vector>

namespace flachead::layout
{
struct LayoutResult
{
    std::vector<Rect> rects;
    Vec2 contentSize;
};

enum class Orientation
{
    Horizontal,
    Vertical,
};

// Abstract layout used by Widgets. BoxLayout and GridLayout implement it.
class LayoutEngine
{
public:
    virtual ~LayoutEngine() = default;
    virtual LayoutResult Layout(const std::vector<Vec2>& preferred, const Vec2& bounds) const = 0;
    virtual Vec2 Measure(const std::vector<Vec2>& preferred) const = 0;
};

std::unique_ptr<LayoutEngine> MakeBox(Orientation orientation);
std::unique_ptr<LayoutEngine> MakeGrid(int columns);
} // namespace flachead::layout
