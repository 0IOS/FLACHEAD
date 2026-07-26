#include "HomeScreen.hpp"

void HomeScreen::Draw(
    flachead::ui::Canvas& canvas,
    int width,
    int height)
{
    m_AlbumArt.Draw(canvas,width,height);

    m_SongInfo.Draw(canvas,width,height);

    m_Progress.Draw(canvas,width,height);

    m_BottomBar.Draw(canvas,width,height);
}