#pragma once

#include "../math/Rect.hpp"

namespace Layout
{
    // Window
    constexpr float WindowWidth  = 900.0f;
    constexpr float WindowHeight = 600.0f;

    // Margins
    constexpr float Padding = 30.0f;

    // Bottom bar
    constexpr float BottomBarHeight = 60.0f;

    // Album Art
    constexpr float AlbumArtSize = 360.0f;

    // Song Info
    constexpr float SongInfoHeight = 80.0f;

    // Progress
    constexpr float ProgressHeight = 20.0f;

    Rect AlbumArt();
    Rect SongInfo();
    Rect ProgressBar();
    Rect BottomBar();
}