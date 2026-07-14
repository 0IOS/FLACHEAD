#include "Layout.hpp"

namespace Layout
{

LayoutMetrics Calculate(
    int windowWidth,
    int windowHeight)
{
    LayoutMetrics m;

    m.windowWidth = static_cast<float>(windowWidth);
    m.windowHeight = static_cast<float>(windowHeight);

    m.padding = windowWidth * 0.04f;

    m.albumSize = windowWidth * 0.45f;

    float maxAlbum = windowHeight * 0.55f;

    if (m.albumSize > maxAlbum)
        m.albumSize = maxAlbum;

    m.songHeight = m.albumSize * 0.18f;

    m.progressHeight = m.albumSize * 0.04f;

    if (m.progressHeight < 6.0f)
        m.progressHeight = 6.0f;

    m.bottomBarHeight = windowHeight * 0.10f;

    return m;
}

Rect AlbumArt(const LayoutMetrics& m)
{
    return Rect(
        (m.windowWidth - m.albumSize) / 2.0f,
        m.padding,
        m.albumSize,
        m.albumSize
    );
}

Rect SongInfo(const LayoutMetrics& m)
{
    return Rect(
        (m.windowWidth - m.albumSize) / 2.0f,
        m.padding + m.albumSize + m.padding,
        m.albumSize,
        m.songHeight
    );
}

Rect ProgressBar(const LayoutMetrics& m)
{
    return Rect(
        (m.windowWidth - m.albumSize) / 2.0f,
        m.padding +
        m.albumSize +
        m.padding +
        m.songHeight +
        m.padding,
        m.albumSize,
        m.progressHeight
    );
}

Rect BottomBar(const LayoutMetrics& m)
{
    return Rect(
        0,
        m.windowHeight - m.bottomBarHeight,
        m.windowWidth,
        m.bottomBarHeight
    );
}

}