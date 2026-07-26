#include "HomeScreen.hpp"

#include "../layout/Layout.hpp"
#include "../layout/LayoutMetrics.hpp"
#include "../math/Color.hpp"

void HomeScreen::Draw(flachead::ui::Canvas& canvas, int windowWidth, int windowHeight)
{
    const LayoutMetrics metrics = flachead::layout::Calculate(windowWidth, windowHeight);

    canvas.DrawRect(flachead::layout::AlbumArt(metrics), Color::White);
    canvas.DrawRect(flachead::layout::SongInfo(metrics), Color::White);
    canvas.DrawRect(flachead::layout::ProgressBar(metrics), Color::White);
    canvas.DrawRect(flachead::layout::BottomBar(metrics), Color::White);
}