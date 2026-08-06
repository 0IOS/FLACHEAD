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

// Create a box layout. `spacing` is the gap between children along the
// main axis. `padding` is applied on all sides of the box. Defaults provide
// a coherent system spacing when callers omit explicit values.
std::unique_ptr<LayoutEngine> MakeBox(Orientation orientation, float spacing = 8.0f, float padding = 12.0f);
// Create a grid layout with `columns`. `spacing` is the gap between cells.
std::unique_ptr<LayoutEngine> MakeGrid(int columns, float spacing = 8.0f);
} // namespace flachead::layout
