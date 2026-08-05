#include "HomeScreen.hpp"

#include "../dap/DapScreen.hpp"
#include "../events/EventBus.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"

#include <algorithm>

namespace flachead::dap
{
namespace
{
const Color kAccent   = Color{124, 58, 237, 255};
const Color kFg       = Color{226, 232, 240, 255};
const Color kFgMuted  = Color{148, 163, 184, 255};
const Color kLine     = Color{30, 36, 51, 255};
const Color kRowSel   = Color{28, 35, 51, 255};
const Color kRowBg    = Color{16, 20, 30, 255};
constexpr float kRowH = 46.0f;
} // namespace

HomeScreen::HomeScreen(const AppContext& context)
    : DapScreen(context, "Home")
{
    Subscribe(flachead::events::Type::TrackChanged);
    Subscribe(flachead::events::Type::PlaybackStarted);
    Subscribe(flachead::events::Type::PlaybackPaused);
    Subscribe(flachead::events::Type::PlaybackResumed);
    Subscribe(flachead::events::Type::PlaybackStopped);
    Subscribe(flachead::events::Type::PlaybackFinished);
    Subscribe(flachead::events::Type::QueueChanged);
}

void HomeScreen::RefreshData()
{
    const auto& playback = *Ctx().playback;
    const auto& track = playback.CurrentTrack();
    m_HasPlayback = track.Valid();
    m_NowPlayingTitle = m_HasPlayback ? track.DisplayTitle() : "Nothing playing";
    m_NowPlayingArtist = m_HasPlayback ? track.DisplayArtist() : "Pick a track from the library";
    m_Playing = playback.IsPlaying();
}

void HomeScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title(),
               m_HasPlayback ? (m_Playing ? "Now playing" : "Paused") : "No playback");

    // Now-playing hero.
    const float heroH = 96.0f;
    const float hy = kStatusH + kHeaderH + 8.0f;
    canvas.FillRoundedRect(Rect{20.0f, hy, static_cast<float>(width) - 40.0f, heroH}, 10.0f,
                           m_HasPlayback ? kRowSel : Color{20, 24, 36, 255});
    canvas.DrawRoundedRect(Rect{20.0f, hy, static_cast<float>(width) - 40.0f, heroH}, 10.0f,
                           m_HasPlayback ? kAccent : kLine);

    const Color art = ArtPlaceholder(m_NowPlayingTitle);
    canvas.FillRoundedRect(Rect{32.0f, hy + 12.0f, 72.0f, 72.0f}, 8.0f, art);
    canvas.DrawTextCentered(Rect{32.0f, hy + 12.0f, 72.0f, 72.0f}, "♪",
                            Color{20, 24, 36, 255}, 30.0f);

    canvas.DrawText(Rect{122.0f, hy + 18.0f, static_cast<float>(width) - 220.0f, 22.0f},
                    m_NowPlayingTitle, Color::White, 18.0f);
    canvas.DrawText(Rect{122.0f, hy + 44.0f, static_cast<float>(width) - 220.0f, 18.0f},
                    m_NowPlayingArtist, kFgMuted, 14.0f);
    canvas.DrawText(Rect{122.0f, hy + 64.0f, 200.0f, 16.0f},
                    m_HasPlayback ? "ENTER: open player" : "Run a scan to import music",
                    kFgMuted, 12.0f);

    // Navigation rows.
    const float listTop = hy + heroH + 10.0f;
    const float maxY = static_cast<float>(height) - kFooterH - 6.0f;
    const int visible = std::max(1, static_cast<int>((maxY - listTop) / kRowH));
    const int startRow = std::max(0, m_SelectedIndex - visible / 2);
    const int endRow = std::min(static_cast<int>(m_Entries.size()), startRow + visible);

    for (int i = startRow; i < endRow; ++i)
    {
        const float y = listTop + static_cast<float>(i - startRow) * kRowH;
        const bool selected = i == m_SelectedIndex;
        const auto& entry = m_Entries[static_cast<std::size_t>(i)];

        canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), kRowH},
                        selected ? kRowSel : kRowBg);
        if (selected)
        {
            canvas.FillRect(Rect{0.0f, y, 4.0f, kRowH}, kAccent);
        }
        canvas.DrawLine(0.0f, y + kRowH, static_cast<float>(width), y + kRowH, kLine);

        canvas.DrawText(Rect{28.0f, y + 13.0f, 36.0f, 22.0f}, entry.icon, kAccent, 18.0f);
        canvas.DrawText(Rect{76.0f, y + 13.0f, 300.0f, 22.0f}, entry.name,
                        selected ? Color::White : kFg, 17.0f);
        canvas.DrawText(Rect{static_cast<float>(width) - 120.0f, y + 14.0f, 90.0f, 18.0f},
                        selected ? "\xe2\x96\xb8  open" : "", kFgMuted, 13.0f);
    }

    DrawFooter(canvas, width, height,
               "UP/DOWN: navigate   ENTER: open   ESC: quit");
}

void HomeScreen::Activate()
{
    if (m_SelectedIndex == 0 && !m_HasPlayback && Ctx().library->SongCount() == 0)
    {
        Ctx().navigate("scan");
        return;
    }
    Ctx().navigate(m_Entries[static_cast<std::size_t>(m_SelectedIndex)].screen);
}

bool HomeScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
    {
        return false;
    }

    switch (event.key.key)
    {
        case SDLK_ESCAPE:
            if (m_OnBack)
            {
                m_OnBack();
            }
            else
            {
                Ctx().goBack();
            }
            return true;
        case SDLK_UP:
            m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
            return true;
        case SDLK_DOWN:
            m_SelectedIndex = std::min(static_cast<int>(m_Entries.size()) - 1,
                                       m_SelectedIndex + 1);
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            Activate();
            return true;
        default:
            break;
    }
    return false;
}
} // namespace flachead::dap
