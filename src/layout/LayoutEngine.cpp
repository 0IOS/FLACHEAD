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
} // namespace flachead::layout
