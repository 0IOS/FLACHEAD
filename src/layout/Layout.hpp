#pragma once

#include "../math/Rect.hpp"

namespace Layout
{
    // Margins
    constexpr float Padding = 30.0f;

    // Album Art
    constexpr float AlbumArtSize = 360.0f;

    // Song Info
    constexpr float SongInfoHeight = 80.0f;

    // Progress
    constexpr float ProgressHeight = 20.0f;

    // Bottom Bar
    constexpr float BottomBarHeight = 60.0f;

    Rect AlbumArt(int windowWidth, int windowHeight);
    Rect SongInfo(int windowWidth, int windowHeight);
    Rect ProgressBar(int windowWidth, int windowHeight);
    Rect BottomBar(int windowWidth, int windowHeight);
}