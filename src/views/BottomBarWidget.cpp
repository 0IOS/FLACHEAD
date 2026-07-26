#include "BottomBarWidget.hpp"

#include "../layout/Layout.hpp"
#include "../layout/LayoutMetrics.hpp"
#include "../math/Color.hpp"

void BottomBarWidget::Draw(
    flachead::ui::Canvas& canvas,
    int width,
    int height)
{
    auto metrics = flachead::layout::Calculate(width,height);

    canvas.DrawRect(
        flachead::layout::BottomBar(metrics),
        Color::White
    );
}