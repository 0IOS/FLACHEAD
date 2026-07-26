#include "Layout.hpp"

namespace flachead::layout
{
LayoutMetrics Calculate(int windowWidth, int windowHeight)
{
    LayoutMetrics metrics{};
    metrics.windowWidth = static_cast<float>(windowWidth);
    metrics.windowHeight = static_cast<float>(windowHeight);
    metrics.padding = windowWidth * 0.04f;
    metrics.albumSize = windowWidth * 0.45f;

    const float maxAlbum = windowHeight * 0.55f;
    if (metrics.albumSize > maxAlbum)
    {
        metrics.albumSize = maxAlbum;
    }

    metrics.songHeight = metrics.albumSize * 0.18f;
    metrics.progressHeight = metrics.albumSize * 0.04f;
    if (metrics.progressHeight < 6.0f)
    {
        metrics.progressHeight = 6.0f;
    }

    metrics.bottomBarHeight = windowHeight * 0.10f;
    return metrics;
}

Rect AlbumArt(const LayoutMetrics& metrics)
{
    return Rect((metrics.windowWidth - metrics.albumSize) / 2.0f,
                metrics.padding,
                metrics.albumSize,
                metrics.albumSize);
}

Rect SongInfo(const LayoutMetrics& metrics)
{
    return Rect((metrics.windowWidth - metrics.albumSize) / 2.0f,
                metrics.padding + metrics.albumSize + metrics.padding,
                metrics.albumSize,
                metrics.songHeight);
}

Rect ProgressBar(const LayoutMetrics& metrics)
{
    return Rect((metrics.windowWidth - metrics.albumSize) / 2.0f,
                metrics.padding + metrics.albumSize + metrics.padding + metrics.songHeight + metrics.padding,
                metrics.albumSize,
                metrics.progressHeight);
}

Rect BottomBar(const LayoutMetrics& metrics)
{
    return Rect(0.0f, metrics.windowHeight - metrics.bottomBarHeight, metrics.windowWidth, metrics.bottomBarHeight);
}
} // namespace flachead::layout