#include "SongInfoWidget.hpp"

#include "../layout/Layout.hpp"
#include "../layout/LayoutMetrics.hpp"
#include "../math/Color.hpp"

void SongInfoWidget::Draw(
    flachead::ui::Canvas& canvas,
    int width,
    int height)
{
    const auto metrics = flachead::layout::Calculate(width, height);
    const auto songRect = flachead::layout::SongInfo(metrics);

    canvas.FillRect(songRect, Color{10, 10, 14, 255});
    canvas.DrawRect(songRect, Color{255, 255, 255, 120});

    canvas.DrawText(Rect{songRect.position.x, songRect.position.y + 6.0f, songRect.size.x, 24.0f},
                    "Nebula Echo", Color::White, 24.0f);
    canvas.DrawText(Rect{songRect.position.x, songRect.position.y + 34.0f, songRect.size.x, 18.0f},
                    "Liora Vale • Midnight Static", Color{200, 200, 200, 255}, 18.0f);
    canvas.DrawText(Rect{songRect.position.x, songRect.position.y + 56.0f, songRect.size.x, 14.0f},
                    "AAC 24-bit • 48 kHz", Color{140, 140, 140, 255}, 14.0f);
}