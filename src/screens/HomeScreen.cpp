#include "HomeScreen.hpp"

#include "../layout/Layout.hpp"
#include "../layout/LayoutMetrics.hpp"
#include "../math/Color.hpp"

void HomeScreen::Draw(
    Canvas& canvas,
    int windowWidth,
    int windowHeight)
{
    LayoutMetrics metrics =
        Layout::Calculate(
            windowWidth,
            windowHeight
        );

    canvas.DrawRect(
        Layout::AlbumArt(metrics),
        Color::White
    );

    canvas.DrawRect(
        Layout::SongInfo(metrics),
        Color::White
    );

    canvas.DrawRect(
        Layout::ProgressBar(metrics),
        Color::White
    );

    canvas.DrawRect(
        Layout::BottomBar(metrics),
        Color::White
    );
}