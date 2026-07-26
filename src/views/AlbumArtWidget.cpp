#include "AlbumArtWidget.hpp"

#include "../layout/Layout.hpp"
#include "../layout/LayoutMetrics.hpp"
#include "../math/Color.hpp"

void AlbumArtWidget::Draw(
    flachead::ui::Canvas& canvas,
    int width,
    int height)
{
    const auto metrics = flachead::layout::Calculate(width, height);
    const auto albumRect = flachead::layout::AlbumArt(metrics);

    canvas.FillRect(albumRect, Color{18, 18, 20, 255});
    canvas.DrawRect(albumRect, Color::White);

    const Rect highlight{albumRect.position.x + 12.0f, albumRect.position.y + 12.0f,
                         albumRect.size.x - 24.0f, albumRect.size.y - 24.0f};
    canvas.DrawRect(highlight, Color{120, 120, 120, 255});
    canvas.DrawText(Rect{albumRect.position.x + 20.0f, albumRect.position.y + 24.0f, 140.0f, 24.0f},
                    "AETHER", Color::White, 20.0f);
}