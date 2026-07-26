#include "BottomBarWidget.hpp"

#include "../layout/Layout.hpp"
#include "../layout/LayoutMetrics.hpp"
#include "../math/Color.hpp"

void BottomBarWidget::Draw(
    flachead::ui::Canvas& canvas,
    int width,
    int height)
{
    const auto metrics = flachead::layout::Calculate(width, height);
    const auto barRect = flachead::layout::BottomBar(metrics);

    canvas.FillRect(barRect, Color{6, 6, 9, 255});
    canvas.DrawRect(barRect, Color{255, 255, 255, 80});

    const Rect prev{barRect.position.x + 24.0f, barRect.position.y + 18.0f, 42.0f, 24.0f};
    const Rect play{barRect.position.x + width * 0.5f - 22.0f, barRect.position.y + 12.0f, 44.0f, 28.0f};
    const Rect next{barRect.position.x + width - 100.0f, barRect.position.y + 18.0f, 42.0f, 24.0f};

    canvas.DrawRect(prev, Color::White);
    canvas.DrawRect(play, Color::White);
    canvas.DrawRect(next, Color::White);
    canvas.DrawText(Rect{barRect.position.x + 24.0f, barRect.position.y + 8.0f, 140.0f, 12.0f}, "◀", Color::White, 14.0f);
    canvas.DrawText(Rect{barRect.position.x + width * 0.5f - 12.0f, barRect.position.y + 12.0f, 24.0f, 12.0f}, "▶", Color::White, 14.0f);
    canvas.DrawText(Rect{barRect.position.x + width - 100.0f, barRect.position.y + 8.0f, 140.0f, 12.0f}, "▶▶", Color::White, 14.0f);
}