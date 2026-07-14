#include "HomeScreen.hpp"

#include "../layout/Layout.hpp"
#include "../math/Color.hpp"

void HomeScreen::Draw(Canvas& canvas)
{
    // Album Art
    canvas.DrawRect(
        Layout::AlbumArt(),
        Color::White
    );

    // Song Information
    canvas.DrawRect(
        Layout::SongInfo(),
        Color::White
    );

    // Progress Bar
    canvas.DrawRect(
        Layout::ProgressBar(),
        Color::White
    );

    // Bottom Navigation Bar
    canvas.DrawRect(
        Layout::BottomBar(),
        Color::White
    );
}