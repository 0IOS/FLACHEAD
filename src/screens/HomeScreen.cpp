#include "HomeScreen.hpp"

#include "../layout/Layout.hpp"
#include "../math/Color.hpp"

void HomeScreen::Draw(
    Canvas& canvas,
    int windowWidth,
    int windowHeight)
{
    canvas.DrawRect(
        Layout::AlbumArt(windowWidth, windowHeight),
        Color::White
    );

    canvas.DrawRect(
        Layout::SongInfo(windowWidth, windowHeight),
        Color::White
    );

    canvas.DrawRect(
        Layout::ProgressBar(windowWidth, windowHeight),
        Color::White
    );

    canvas.DrawRect(
        Layout::BottomBar(windowWidth, windowHeight),
        Color::White
    );
}