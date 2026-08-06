#include "LayoutEngine.hpp"

#include "BoxLayout.hpp"
#include "GridLayout.hpp"

namespace flachead::layout
{
std::unique_ptr<LayoutEngine> MakeBox(Orientation orientation)
{
    return std::make_unique<BoxLayout>(orientation);
}

std::unique_ptr<LayoutEngine> MakeGrid(int columns)
{
    auto grid = std::make_unique<GridLayout>();
    grid->Columns(columns);
    return grid;
}

std::unique_ptr<LayoutEngine> MakeBox(Orientation orientation, float spacing, float padding)
{
    auto box = std::make_unique<BoxLayout>(orientation);
    box->Spacing(spacing);
    box->PaddingAll(padding);
    box->MainAxis(AxisAlign::Start);
    box->CrossAxis(CrossAlign::Stretch);
    return box;
}

std::unique_ptr<LayoutEngine> MakeGrid(int columns, float spacing)
{
    auto grid = std::make_unique<GridLayout>();
    grid->Columns(columns);
    grid->Spacing(spacing);
    return grid;
}
} // namespace flachead::layout
