#pragma once

#include "LayoutMetrics.hpp"
#include "../math/Rect.hpp"

namespace Layout
{
    LayoutMetrics Calculate(
        int windowWidth,
        int windowHeight);

    Rect AlbumArt(const LayoutMetrics& m);

    Rect SongInfo(const LayoutMetrics& m);

    Rect ProgressBar(const LayoutMetrics& m);

    Rect BottomBar(const LayoutMetrics& m);
}