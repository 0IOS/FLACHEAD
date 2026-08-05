#pragma once

#include "LayoutMetrics.hpp"
#include "../math/Rect.hpp"

namespace flachead::layout
{
class Layout
{
public:
    virtual ~Layout() = default;
    virtual void Measure(const Rect& bounds) = 0;
    virtual void Arrange(const Rect& bounds) = 0;
};

LayoutMetrics Calculate(int windowWidth, int windowHeight);
Rect AlbumArt(const LayoutMetrics& metrics);
Rect SongInfo(const LayoutMetrics& metrics);
Rect ProgressBar(const LayoutMetrics& metrics);
Rect BottomBar(const LayoutMetrics& metrics);
} // namespace flachead::layout
