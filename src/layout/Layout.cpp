#include "Layout.hpp"

namespace Layout
{

Rect AlbumArt(int windowWidth, int /*windowHeight*/)
{
    float x = (windowWidth - AlbumArtSize) / 2.0f;
    float y = Padding;

    return Rect(x, y, AlbumArtSize, AlbumArtSize);
}

Rect SongInfo(int windowWidth, int /*windowHeight*/)
{
    float x = (windowWidth - AlbumArtSize) / 2.0f;

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

Rect ProgressBar(int windowWidth, int /*windowHeight*/)
{
    float x = (windowWidth - AlbumArtSize) / 2.0f;

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

Rect BottomBar(int windowWidth, int windowHeight)
{
    return Rect(
        0,
        windowHeight - BottomBarHeight,
        windowWidth,
        BottomBarHeight
    );
}

}