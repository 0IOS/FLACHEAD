#include "ProgressWidget.hpp"

#include "../layout/Layout.hpp"
#include "../layout/LayoutMetrics.hpp"
#include "../math/Color.hpp"

void ProgressWidget::Draw(
    flachead::ui::Canvas& canvas,
    int width,
    int height)
{
    const auto metrics = flachead::layout::Calculate(width, height);
    const auto progressRect = flachead::layout::ProgressBar(metrics);

    const Rect track{progressRect.position.x, progressRect.position.y, progressRect.size.x, progressRect.size.y};
    canvas.FillRect(track, Color{25, 25, 30, 255});
    canvas.DrawRect(track, Color{255, 255, 255, 80});

    const Rect fill{progressRect.position.x, progressRect.position.y, progressRect.size.x * 0.62f, progressRect.size.y};
    canvas.FillRect(fill, Color::White);

    canvas.DrawText(Rect{track.position.x, track.position.y - 24.0f, 80.0f, 20.0f}, "03:12", Color::White, 16.0f);
    canvas.DrawText(Rect{track.position.x + track.size.x - 80.0f, track.position.y - 24.0f, 80.0f, 20.0f}, "04:58", Color{170, 170, 170, 255}, 16.0f);
}