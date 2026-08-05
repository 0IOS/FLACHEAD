#include "PlaybackScreens.hpp"

#include "../events/EventBus.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"
#include "../playback/QueueManager.hpp"

#include <algorithm>
#include <cstdio>

namespace flachead::dap
{
namespace
{
const Color kAccent = Color{124, 58, 237, 255};
const Color kFg     = Color{226, 232, 240, 255};
const Color kFgMuted = Color{148, 163, 184, 255};
const Color kLine   = Color{30, 36, 51, 255};
const Color kControlBg = Color{22, 28, 40, 255};
const Color kControlSel = Color{124, 58, 237, 255};
const char* kPlayHints =
    "SPACE: play/pause   LEFT/RIGHT: seek   UP/DOWN: volume   P/N: prev/next   "
    "S: shuffle   R: repeat   F: favorite   Q: queue   ESC: back";
} // namespace

// ===========================================================================
// NowPlayingScreen
// ===========================================================================
NowPlayingScreen::NowPlayingScreen(const AppContext& context)
    : DapScreen(context, "Now Playing")
{
    Subscribe(flachead::events::Type::TrackChanged);
    Subscribe(flachead::events::Type::TrackLoaded);
    Subscribe(flachead::events::Type::TrackProgress);
    Subscribe(flachead::events::Type::PlaybackStarted);
    Subscribe(flachead::events::Type::PlaybackPaused);
    Subscribe(flachead::events::Type::PlaybackResumed);
    Subscribe(flachead::events::Type::PlaybackStopped);
    Subscribe(flachead::events::Type::PlaybackFinished);
    Subscribe(flachead::events::Type::VolumeChanged);
    Subscribe(flachead::events::Type::QueueChanged);
    Subscribe(flachead::events::Type::ShuffleChanged);
    Subscribe(flachead::events::Type::RepeatChanged);
}

void NowPlayingScreen::DrawArt(flachead::ui::Canvas& canvas, int width) const
{
    const auto& playback = *Ctx().playback;
    const auto& track = playback.CurrentTrack();

    constexpr float artSize = 180.0f;
    const float ax = (static_cast<float>(width) - artSize) * 0.5f;
    const float ay = kStatusH + kHeaderH + 18.0f;

    const std::string seed = track.Valid() ? (track.artPath.empty() ? track.path : track.artPath)
                                           : "none";
    const Color art = ArtPlaceholder(seed);

    canvas.FillRoundedRect(Rect{ax, ay, artSize, artSize}, 12.0f, art);
    canvas.DrawRoundedRect(Rect{ax, ay, artSize, artSize}, 12.0f, Color{40, 48, 66, 255});
    canvas.DrawTextCentered(Rect{ax, ay + artSize * 0.38f, artSize, 40.0f}, "♪", kControlBg,
                            64.0f);

    if (track.Valid() && !track.artPath.empty())
    {
        canvas.DrawTextCentered(Rect{ax, ay + artSize + 6.0f, artSize, 16.0f},
                                "cached art", kFgMuted, 11.0f);
    }
}

void NowPlayingScreen::DrawTransport(flachead::ui::Canvas& canvas, int width, int height) const
{
    const auto& playback = *Ctx().playback;
    const auto& track = playback.CurrentTrack();

    const double pos = playback.PositionSeconds();
    const double dur = playback.DurationSeconds();

    // Seek bar.
    constexpr float barW = 460.0f;
    const float bx = (static_cast<float>(width) - barW) * 0.5f;
    const float by = kStatusH + kHeaderH + 226.0f;
    canvas.FillRoundedRect(Rect{bx, by, barW, 6.0f}, 3.0f, kControlBg);
    const float frac = dur > 0.0 ? static_cast<float>(std::clamp(pos / dur, 0.0, 1.0)) : 0.0f;
    if (frac > 0.0f)
    {
        canvas.FillRoundedRect(Rect{bx, by, barW * frac, 6.0f}, 3.0f, kAccent);
    }
    canvas.DrawText(Rect{bx, by + 12.0f, 70.0f, 16.0f}, FormatDuration(pos), kFg, 13.0f);
    canvas.DrawText(Rect{bx + barW - 70.0f, by + 12.0f, 70.0f, 16.0f},
                    dur > 0.0 ? "-" + FormatDuration(dur - pos) : "--:--", kFgMuted, 13.0f);

    // Transport buttons.
    constexpr float btnY = 340.0f;
    constexpr float btnH = 56.0f;
    const float prevW = 96.0f;
    const float playW = 112.0f;
    const float nextW = 96.0f;
    const float gap = 22.0f;
    const float totalW = prevW + playW + nextW + 2.0f * gap;
    const float x0 = (static_cast<float>(width) - totalW) * 0.5f;

    const bool playing = playback.IsPlaying();

    const Rect prevRect{x0, btnY, prevW, btnH};
    const Rect playRect{x0 + prevW + gap, btnY, playW, btnH};
    const Rect nextRect{x0 + prevW + gap + playW + gap, btnY, nextW, btnH};

    canvas.FillRoundedRect(prevRect, 10.0f, m_Focus == 0 ? kControlSel : kControlBg);
    canvas.FillRoundedRect(playRect, 10.0f, m_Focus == 1 ? kControlSel : kControlBg);
    canvas.FillRoundedRect(nextRect, 10.0f, m_Focus == 2 ? kControlSel : kControlBg);

    const Color btnFg = Color{255, 255, 255, 255};
    canvas.DrawTextCentered(prevRect, "⏮", btnFg, 26.0f);
    canvas.DrawTextCentered(playRect, playing ? "⏸" : "▶", btnFg, 30.0f);
    canvas.DrawTextCentered(nextRect, "⏭", btnFg, 26.0f);

    // Shuffle / repeat indicators below the transport.
    const float infoY = btnY + btnH + 14.0f;
    const auto& queue = playback.Queue();
    std::string state;
    if (queue.Shuffle())
    {
        state += "Shuffle on";
    }
    if (queue.Repeat() != flachead::playback::RepeatMode::Off)
    {
        if (!state.empty())
        {
            state += "  ·  ";
        }
        state += queue.Repeat() == flachead::playback::RepeatMode::One ? "Repeat one"
                                                                       : "Repeat all";
    }
    if (state.empty())
    {
        state = "Shuffle off  ·  Repeat off";
    }
    canvas.DrawTextCentered(Rect{0.0f, infoY, static_cast<float>(width), 18.0f}, state,
                            kFgMuted, 13.0f);

    // Favorite + metadata.
    if (track.Valid())
    {
        std::string meta = track.DisplayArtist() + " — " + track.DisplayAlbum();
        if (track.year > 0)
        {
            meta += " (" + std::to_string(track.year) + ")";
        }
        canvas.DrawTextCentered(Rect{0.0f, infoY + 22.0f, static_cast<float>(width), 16.0f},
                                meta, kFg, 14.0f);
        const float favY = infoY + 44.0f;
        canvas.DrawTextCentered(Rect{0.0f, favY, static_cast<float>(width), 20.0f},
                                track.favorite ? "♥ Favorite" : "♡ Favorite",
                                track.favorite ? Color{244, 63, 94, 255} : kFgMuted, 15.0f);
    }
}

void NowPlayingScreen::DrawVolume(flachead::ui::Canvas& canvas, int width, int height) const
{
    const float vol = Ctx().playback->Volume();
    constexpr float barW = 200.0f;
    const float bx = (static_cast<float>(width) - barW) * 0.5f;
    const float by = static_cast<float>(height) - kFooterH - 60.0f;

    canvas.DrawText(Rect{bx - 120.0f, by - 2.0f, 110.0f, 20.0f}, "Volume", kFgMuted, 14.0f);

    canvas.FillRoundedRect(Rect{bx, by, barW, 6.0f}, 3.0f, kControlBg);
    if (vol > 0.0f)
    {
        canvas.FillRoundedRect(Rect{bx, by, barW * vol, 6.0f}, 3.0f, Color{34, 211, 238, 255});
    }

    char buf[8];
    std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(vol * 100.0f + 0.5f));
    canvas.DrawText(Rect{bx + barW + 12.0f, by - 4.0f, 50.0f, 20.0f}, buf, kFg, 14.0f);
}

void NowPlayingScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title());

    const auto& playback = *Ctx().playback;
    if (playback.Queue().IsEmpty())
    {
        DrawEmpty(canvas, width, height, "Nothing queued — pick a track from the Library");
        DrawFooter(canvas, width, height, "ESC: back");
        return;
    }

    const auto& track = playback.CurrentTrack();
    canvas.DrawTextCentered(Rect{0.0f, kStatusH + 6.0f, static_cast<float>(width), 28.0f},
                            track.DisplayTitle(), Color::White, 22.0f);

    DrawArt(canvas, width);
    DrawTransport(canvas, width, height);
    DrawVolume(canvas, width, height);
    DrawFooter(canvas, width, height, kPlayHints);
}

void NowPlayingScreen::ToggleFavorite()
{
    const auto& track = Ctx().playback->CurrentTrack();
    if (track.Valid())
    {
        Ctx().library->ToggleFavorite(track.id);
    }
}

bool NowPlayingScreen::HandleEvent(const SDL_Event& event)
{
    auto& playback = *Ctx().playback;
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_ESCAPE:
                Ctx().goBack();
                return true;
            case SDLK_SPACE:
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                playback.Toggle();
                return true;
            case SDLK_LEFT:
                playback.Seek(playback.PositionSeconds() - 5.0);
                return true;
            case SDLK_RIGHT:
                playback.Seek(playback.PositionSeconds() + 5.0);
                return true;
            case SDLK_UP:
                playback.SetVolume(playback.Volume() + 0.05f);
                return true;
            case SDLK_DOWN:
                playback.SetVolume(playback.Volume() - 0.05f);
                return true;
            case SDLK_P:
                playback.Previous();
                return true;
            case SDLK_N:
                playback.Next();
                return true;
            case SDLK_S:
                playback.ToggleShuffle();
                return true;
            case SDLK_R:
                playback.ToggleRepeat();
                return true;
            case SDLK_F:
                ToggleFavorite();
                return true;
            case SDLK_Q:
                Ctx().navigate("queue");
                return true;
            case SDLK_TAB:
                m_Focus = (m_Focus + 1) % 3;
                return true;
            default:
                break;
        }
        return false;
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        const float mx = static_cast<float>(event.button.x);
        const float my = static_cast<float>(event.button.y);

        constexpr float btnY = 340.0f;
        constexpr float btnH = 56.0f;
        const float prevW = 96.0f;
        const float playW = 112.0f;
        const float nextW = 96.0f;
        const float gap = 22.0f;
        const float totalW = prevW + playW + nextW + 2.0f * gap;
        const float x0 = (static_cast<float>(ViewWidth()) - totalW) * 0.5f;

        if (my >= btnY && my <= btnY + btnH)
        {
            if (mx >= x0 && mx <= x0 + prevW)
            {
                playback.Previous();
                return true;
            }
            if (mx >= x0 + prevW + gap && mx <= x0 + prevW + gap + playW)
            {
                playback.Toggle();
                return true;
            }
            if (mx >= x0 + prevW + gap + playW + gap && mx <= x0 + totalW)
            {
                playback.Next();
                return true;
            }
        }

        // Volume bar (uses current window width, best-effort hit region).
        constexpr float barW = 200.0f;
        const float bx = (static_cast<float>(ViewWidth()) - barW) * 0.5f;
        const float by = static_cast<float>(ViewHeight()) - kFooterH - 60.0f;
        if (my >= by - 4.0f && my <= by + 10.0f && mx >= bx && mx <= bx + barW)
        {
            const float frac = std::clamp((mx - bx) / barW, 0.0f, 1.0f);
            playback.SetVolume(frac);
            return true;
        }
    }
    return false;
}

// ===========================================================================
// QueueScreen
// ===========================================================================
QueueScreen::QueueScreen(const AppContext& context)
    : DapScreen(context, "Queue")
{
    Subscribe(flachead::events::Type::QueueChanged);
    Subscribe(flachead::events::Type::TrackChanged);
    Subscribe(flachead::events::Type::ShuffleChanged);
    Subscribe(flachead::events::Type::RepeatChanged);
}

void QueueScreen::RefreshData()
{
    m_Tracks = Ctx().playback->Queue().Tracks();
    m_SelectedIndex = std::clamp(m_SelectedIndex, 0, std::max(0, static_cast<int>(m_Tracks.size()) - 1));
}

void QueueScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);

    const auto& playback = *Ctx().playback;
    const auto& queue = playback.Queue();

    std::string subtitle = std::to_string(queue.Size()) + " track(s)";
    if (queue.Shuffle())
    {
        subtitle += "  ·  shuffle";
    }
    if (queue.Repeat() != flachead::playback::RepeatMode::Off)
    {
        subtitle += queue.Repeat() == flachead::playback::RepeatMode::One ? "  ·  repeat one"
                                                                          : "  ·  repeat all";
    }
    DrawHeader(canvas, width, Title(), subtitle);

    if (m_Tracks.empty())
    {
        DrawEmpty(canvas, width, height, "Queue is empty");
        DrawFooter(canvas, width, height, "ESC: back");
        return;
    }

    constexpr float rowH = 44.0f;
    const float top = kStatusH + kHeaderH + 6.0f;
    const float maxY = static_cast<float>(height) - kFooterH - 6.0f;
    const int current = playback.CurrentIndex();

    const int visible = std::max(1, static_cast<int>((maxY - top) / rowH));
    const int startRow = std::max(0, m_SelectedIndex - visible / 2);
    const int endRow = std::min(static_cast<int>(m_Tracks.size()), startRow + visible);

    for (int i = startRow; i < endRow; ++i)
    {
        const float y = top + static_cast<float>(i - startRow) * rowH;
        std::string num;
        if (i == current)
        {
            num = "▶ ";
        }
        else
        {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d.", i + 1);
            num = buf;
        }
        const auto& t = m_Tracks[static_cast<std::size_t>(i)];
        const bool sel = i == m_SelectedIndex;
        canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), rowH},
                        sel ? Color{28, 35, 51, 255} : Color{16, 20, 30, 255});
        if (sel)
        {
            canvas.FillRect(Rect{0.0f, y, 4.0f, rowH}, kAccent);
        }
        canvas.DrawLine(0.0f, y + rowH, static_cast<float>(width), y + rowH, kLine);

        canvas.DrawText(Rect{16.0f, y + 12.0f, 60.0f, 20.0f}, num,
                        i == current ? Color{34, 211, 238, 255} : Color{100, 116, 139, 255},
                        15.0f);
        canvas.DrawText(Rect{80.0f, y + 12.0f, static_cast<float>(width) - 260.0f, 20.0f},
                        t.DisplayTitle(), sel ? Color::White : kFg, 16.0f);
        canvas.DrawText(Rect{static_cast<float>(width) - 180.0f, y + 12.0f, 90.0f, 20.0f},
                        t.DisplayArtist(), kFgMuted, 13.0f);
        canvas.DrawText(Rect{static_cast<float>(width) - 90.0f, y + 12.0f, 70.0f, 20.0f},
                        FormatDuration(t.duration), kFgMuted, 13.0f);
    }

    DrawFooter(canvas, width, height,
               "UP/DOWN: select   ENTER: remove   C: clear queue   "
               "S: shuffle   R: repeat   ESC: back");
}

bool QueueScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
    {
        return false;
    }

    auto& playback = *Ctx().playback;
    switch (event.key.key)
    {
        case SDLK_ESCAPE:
            Ctx().goBack();
            return true;
        case SDLK_UP:
            m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
            return true;
        case SDLK_DOWN:
            m_SelectedIndex = std::min(static_cast<int>(m_Tracks.size()) - 1, m_SelectedIndex + 1);
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (!m_Tracks.empty() && m_SelectedIndex >= 0 &&
                m_SelectedIndex < static_cast<int>(m_Tracks.size()))
            {
                playback.RemoveFromQueue(m_SelectedIndex);
            }
            return true;
        case SDLK_C:
            playback.ClearQueue();
            return true;
        case SDLK_S:
            playback.ToggleShuffle();
            return true;
        case SDLK_R:
            playback.ToggleRepeat();
            return true;
        case SDLK_N:
            playback.Next();
            return true;
        default:
            break;
    }
    return false;
}

// ===========================================================================
// ScanScreen
// ===========================================================================
ScanScreen::ScanScreen(const AppContext& context)
    : DapScreen(context, "Library Scan")
{
    Subscribe(flachead::events::Type::LibraryScanStarted,
              [this](const flachead::events::Event&) {
                  m_Scanning = true;
                  m_Processed = 0;
                  m_Total = 0;
              });
    Subscribe(flachead::events::Type::LibraryScanProgress,
              [this](const flachead::events::Event& event) {
                  m_Processed = static_cast<int>(event.intValue);
                  m_Total = static_cast<int>(event.doubleValue);
              });
    Subscribe(flachead::events::Type::LibraryScanFinished,
              [this](const flachead::events::Event& event) {
                  m_Scanning = false;
                  m_Processed = static_cast<int>(event.intValue);
                  m_LastCount = m_Processed;
              });
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void ScanScreen::RefreshData()
{
    m_Scanning = Ctx().library->IsScanning();
    m_LastCount = Ctx().library->LastScanCount();
    m_Focus = m_Scanning ? Action::Cancel : Action::Start;
}

void ScanScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title());

    float y = kStatusH + kHeaderH + 30.0f;

    canvas.DrawText(Rect{60.0f, y, static_cast<float>(width) - 120.0f, 22.0f},
                    m_Scanning ? "Scanning your music library…" : "Library scanner",
                    Color::White, 18.0f);
    y += 30.0f;

    std::string roots;
    for (std::size_t i = 0; i < Ctx().scanRoots.size(); ++i)
    {
        if (i > 0)
        {
            roots += "  ·  ";
        }
        roots += Ctx().scanRoots[i];
    }
    canvas.DrawText(Rect{60.0f, y, static_cast<float>(width) - 120.0f, 18.0f},
                    roots.empty() ? "No scan folders configured" : "Folders: " + roots,
                    Color{148, 163, 184, 255}, 13.0f);
    y += 40.0f;

    // Progress bar.
    constexpr float barW = 480.0f;
    const float bx = (static_cast<float>(width) - barW) * 0.5f;
    canvas.FillRoundedRect(Rect{bx, y, barW, 10.0f}, 5.0f, Color{22, 28, 40, 255});
    const float frac = m_Total > 0 ? static_cast<float>(m_Processed) / static_cast<float>(m_Total)
                                   : (m_Scanning ? 0.0f : 1.0f);
    if (frac > 0.0f)
    {
        canvas.FillRoundedRect(Rect{bx, y, barW * frac, 10.0f}, 5.0f,
                               Color{124, 58, 237, 255});
    }

    char status[64];
    if (m_Scanning)
    {
        std::snprintf(status, sizeof(status), "%d / %d tracks", m_Processed, m_Total);
    }
    else
    {
        std::snprintf(status, sizeof(status), "Library ready — %d track(s)", m_LastCount);
    }
    canvas.DrawTextCentered(Rect{0.0f, y + 16.0f, static_cast<float>(width), 18.0f}, status,
                            Color{148, 163, 184, 255}, 14.0f);
    y += 70.0f;

    // Action buttons.
    const char* label = m_Scanning ? "Cancel scan" : "Start scan";
    const bool selected = true; // single action at a time
    const Rect btn{static_cast<float>(width) * 0.5f - 120.0f, y, 240.0f, 46.0f};
    canvas.FillRoundedRect(btn, 8.0f, selected ? Color{124, 58, 237, 255} : Color{22, 28, 40, 255});
    canvas.DrawTextCentered(btn, label, Color::White, 17.0f);
    y += 70.0f;

    canvas.DrawTextCentered(
        Rect{0.0f, y, static_cast<float>(width), 30.0f},
        "New files are imported automatically; deleted files are pruned.",
        Color{100, 116, 139, 255}, 13.0f);

    DrawFooter(canvas, width, height, "ENTER: " + std::string(m_Scanning ? "cancel" : "start") +
                                          "   ESC: back");
}

void ScanScreen::Activate()
{
    if (m_Scanning)
    {
        Ctx().library->CancelScan();
    }
    else
    {
        Ctx().library->StartScan(Ctx().scanRoots);
    }
}

bool ScanScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_ESCAPE:
                Ctx().goBack();
                return true;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
            case SDLK_SPACE:
                Activate();
                return true;
            default:
                break;
        }
    }
    else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
    {
        const float mx = static_cast<float>(event.button.x);
        const float my = static_cast<float>(event.button.y);
        const float cx = static_cast<float>(ViewWidth()) * 0.5f;
        const float by = kStatusH + kHeaderH + 30.0f + 30.0f + 40.0f + 70.0f;
        if (mx >= cx - 120.0f && mx <= cx + 120.0f && my >= by && my <= by + 46.0f)
        {
            Activate();
            return true;
        }
    }
    return false;
}
} // namespace flachead::dap
