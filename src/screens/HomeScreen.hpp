#pragma once

#include "../ui/Canvas.hpp"

#include "../components/AlbumArtWidget.hpp"
#include "../components/SongInfoWidget.hpp"
#include "../components/ProgressWidget.hpp"
#include "../components/BottomBarWidget.hpp"

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