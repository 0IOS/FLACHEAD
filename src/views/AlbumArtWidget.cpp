#include "AlbumArtWidget.hpp"

#include "../layout/Layout.hpp"
#include "../layout/LayoutMetrics.hpp"
#include "../math/Color.hpp"

void AlbumArtWidget::Draw(
    flachead::ui::Canvas& canvas,
    int width,
    int height)
{
    auto metrics = flachead::layout::Calculate(width,height);

    canvas.DrawRect(
        flachead::layout::AlbumArt(metrics),
        Color::White
    );
}