#include "Layout.hpp"

namespace Layout
{

Rect AlbumArt()
{
    float x = (WindowWidth - AlbumArtSize) / 2.0f;
    float y = Padding;

    return Rect(x, y, AlbumArtSize, AlbumArtSize);
}

Rect SongInfo()
{
    float x = (WindowWidth - AlbumArtSize) / 2.0f;

    float y =
        Padding +
        AlbumArtSize +
        20.0f;

    return Rect(
        x,
        y,
        AlbumArtSize,
        SongInfoHeight
    );
}

Rect ProgressBar()
{
    float x = (WindowWidth - AlbumArtSize) / 2.0f;

    float y =
        Padding +
        AlbumArtSize +
        SongInfoHeight +
        40.0f;

    return Rect(
        x,
        y,
        AlbumArtSize,
        ProgressHeight
    );
}

Rect BottomBar()
{
    return Rect(
        0,
        WindowHeight - BottomBarHeight,
        WindowWidth,
        BottomBarHeight
    );
}

}