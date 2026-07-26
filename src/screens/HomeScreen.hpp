#pragma once

#include "../ui/Canvas.hpp"

#include "../views/AlbumArtWidget.hpp"
#include "../views/SongInfoWidget.hpp"
#include "../views/ProgressWidget.hpp"
#include "../views/BottomBarWidget.hpp"

class HomeScreen
{
public:
    void Draw(flachead::ui::Canvas& canvas,
              int windowWidth,
              int windowHeight);

private:
    AlbumArtWidget m_AlbumArt;
    SongInfoWidget m_SongInfo;
    ProgressWidget m_Progress;
    BottomBarWidget m_BottomBar;
};