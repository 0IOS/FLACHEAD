#include "PlaylistScreens.hpp"

#include "../events/EventBus.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"

#include <algorithm>
#include <cstdio>

namespace flachead::dap
{
namespace
{
const Color kAccent  = Color{124, 58, 237, 255};
const Color kFg      = Color{226, 232, 240, 255};
const Color kFgMuted = Color{148, 163, 184, 255};
const Color kLine    = Color{30, 36, 51, 255};
const Color kRowSel  = Color{28, 35, 51, 255};
const Color kRowBg   = Color{16, 20, 30, 255};
constexpr float kListRowH = 44.0f;
constexpr float kListTop = kStatusH + kHeaderH + 6.0f;

void DrawPlaylistRow(flachead::ui::Canvas& canvas, int width, float y,
                     const std::string& name, const std::string& meta, bool selected)
{
    canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), kListRowH},
                    selected ? kRowSel : kRowBg);
    if (selected)
    {
        canvas.FillRect(Rect{0.0f, y, 4.0f, kListRowH}, kAccent);
    }
    canvas.DrawLine(0.0f, y + kListRowH, static_cast<float>(width), y + kListRowH, kLine);

    canvas.DrawText(Rect{22.0f, y + 8.0f, static_cast<float>(width) - 220.0f, 20.0f}, name,
                    selected ? Color::White : kFg, 16.0f);
    canvas.DrawText(Rect{22.0f, y + 28.0f, static_cast<float>(width) - 220.0f, 14.0f}, meta,
                    kFgMuted, 12.0f);
}
} // namespace

// ===========================================================================
// PlaylistsScreen
// ===========================================================================
PlaylistsScreen::PlaylistsScreen(const AppContext& context)
    : DapScreen(context, "Playlists")
{
    Subscribe(flachead::events::Type::PlaylistChanged);
}

void PlaylistsScreen::RefreshData()
{
    m_Playlists = Ctx().playlists->ListPlaylists();
    // Selection index 0 is the "+ New playlist" row, so clamp to [0, size].
    m_SelectedIndex = std::clamp(m_SelectedIndex, 0, static_cast<int>(m_Playlists.size()));
}

void PlaylistsScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title(), std::to_string(m_Playlists.size()) + " playlist(s)");

    const float maxY = static_cast<float>(height) - kFooterH - 6.0f;
    const int visible = std::max(1, static_cast<int>((maxY - kListTop) / kListRowH));
    const int startRow = std::max(0, m_SelectedIndex - visible / 2);
    const int endRow = std::min(static_cast<int>(m_Playlists.size()) + 1, startRow + visible);

    for (int i = startRow; i < endRow; ++i)
    {
        const float y = kListTop + static_cast<float>(i - startRow) * kListRowH;
        if (i == 0)
        {
            DrawPlaylistRow(canvas, width, y, "+  New playlist",
                            "Create a new empty playlist", m_SelectedIndex == 0);
            continue;
        }
        const auto& pl = m_Playlists[static_cast<std::size_t>(i - 1)];
        DrawPlaylistRow(canvas, width, y, pl.name,
                        std::to_string(pl.trackCount) + " track(s)", m_SelectedIndex == i);
    }

    DrawFooter(canvas, width, height,
               "UP/DOWN: select   ENTER: open / new   D: delete   ESC: back");
}

bool PlaylistsScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
    {
        return false;
    }

    auto& playlists = *Ctx().playlists;
    switch (event.key.key)
    {
        case SDLK_ESCAPE:
            Ctx().goBack();
            return true;
        case SDLK_UP:
            m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
            return true;
        case SDLK_DOWN:
            m_SelectedIndex = std::min(static_cast<int>(m_Playlists.size()), m_SelectedIndex + 1);
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (m_SelectedIndex == 0)
            {
                char name[64];
                std::snprintf(name, sizeof(name), "Playlist %d",
                              static_cast<int>(m_Playlists.size()) + 1);
                const auto id = playlists.CreatePlaylist(name);
                Ctx().pending.playlistId = id;
                Ctx().navigate("playlist");
            }
            else if (m_SelectedIndex - 1 < static_cast<int>(m_Playlists.size()))
            {
                const auto& pl = m_Playlists[static_cast<std::size_t>(m_SelectedIndex - 1)];
                Ctx().pending.playlistId = pl.id;
                Ctx().navigate("playlist");
            }
            return true;
        case SDLK_D:
            if (m_SelectedIndex > 0 &&
                m_SelectedIndex - 1 < static_cast<int>(m_Playlists.size()))
            {
                const auto& pl = m_Playlists[static_cast<std::size_t>(m_SelectedIndex - 1)];
                playlists.DeletePlaylist(pl.id);
            }
            return true;
        default:
            break;
    }
    return false;
}

// ===========================================================================
// PlaylistScreen
// ===========================================================================
PlaylistScreen::PlaylistScreen(const AppContext& context)
    : DapScreen(context, "Playlist")
{
    Subscribe(flachead::events::Type::PlaylistChanged);
    Subscribe(flachead::events::Type::QueueChanged);
}

void PlaylistScreen::RefreshData()
{
    m_PlaylistId = Ctx().pending.playlistId;
    m_PlaylistName = Ctx().playlists->PlaylistName(m_PlaylistId);
    m_Songs.clear();
    const auto ids = Ctx().playlists->TrackIds(m_PlaylistId);
    for (const auto id : ids)
    {
        const auto song = Ctx().library->FindById(id);
        if (song.Valid())
        {
            m_Songs.push_back(song);
        }
    }
    m_SelectedIndex = std::clamp(m_SelectedIndex, 0, static_cast<int>(m_Songs.size()));
}

void PlaylistScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);

    std::string subtitle = m_PlaylistName.empty() ? "Playlist" : m_PlaylistName;
    subtitle += "  ·  " + std::to_string(m_Songs.size()) + " track(s)";
    DrawHeader(canvas, width, Title(), subtitle);

    // "Play playlist" action row.
    const float py = kStatusH + kHeaderH + 6.0f;
    canvas.FillRect(Rect{0.0f, py, static_cast<float>(width), kListRowH},
                    m_SelectedIndex == 0 ? kRowSel : kRowBg);
    if (m_SelectedIndex == 0)
    {
        canvas.FillRect(Rect{0.0f, py, 4.0f, kListRowH}, kAccent);
    }
    canvas.DrawLine(0.0f, py + kListRowH, static_cast<float>(width), py + kListRowH, kLine);
    canvas.DrawText(Rect{22.0f, py + 12.0f, 300.0f, 20.0f}, "▶ Play playlist",
                    m_SelectedIndex == 0 ? Color::White : Color{52, 211, 153, 255}, 16.0f);

    if (m_Songs.empty())
    {
        DrawEmpty(canvas, width, height, "This playlist is empty");
        DrawFooter(canvas, width, height,
                   "Add songs from a song list   R: remove   ESC: back");
        return;
    }

    const float maxY = static_cast<float>(height) - kFooterH - 6.0f;
    const float top = py + kListRowH + 2.0f;
    const int visible = std::max(1, static_cast<int>((maxY - top) / kListRowH));
    const int sel = std::max(0, m_SelectedIndex - 1);
    const int startRow = std::max(0, sel - visible / 2);
    const int endRow = std::min(static_cast<int>(m_Songs.size()), startRow + visible);

    const auto& currentPath = Ctx().playback->CurrentTrack().path;
    for (int i = startRow; i < endRow; ++i)
    {
        const float y = top + static_cast<float>(i - startRow) * kListRowH;
        const auto& song = m_Songs[static_cast<std::size_t>(i)];
        const bool selected = i == sel;
        canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), kListRowH},
                        selected ? kRowSel : kRowBg);
        if (selected)
        {
            canvas.FillRect(Rect{0.0f, y, 4.0f, kListRowH}, kAccent);
        }
        canvas.DrawLine(0.0f, y + kListRowH, static_cast<float>(width), y + kListRowH, kLine);

        char num[8];
        std::snprintf(num, sizeof(num), "%d.", i + 1);
        canvas.DrawText(Rect{16.0f, y + 12.0f, 60.0f, 20.0f},
                        (!currentPath.empty() && song.path == currentPath) ? "▶" : num,
                        (!currentPath.empty() && song.path == currentPath) ? Color{34, 211, 238, 255}
                                                                          : Color{100, 116, 139, 255},
                        15.0f);
        canvas.DrawText(Rect{80.0f, y + 12.0f, static_cast<float>(width) - 340.0f, 20.0f},
                        song.DisplayTitle(), selected ? Color::White : kFg, 16.0f);
        canvas.DrawText(Rect{static_cast<float>(width) - 260.0f, y + 12.0f, 170.0f, 20.0f},
                        song.DisplayArtist(), kFgMuted, 13.0f);
        canvas.DrawText(Rect{static_cast<float>(width) - 90.0f, y + 12.0f, 70.0f, 20.0f},
                        DapScreen::FormatDuration(song.duration), kFgMuted, 13.0f);
    }

    DrawFooter(canvas, width, height,
               "UP/DOWN: select   ENTER: play   R: remove from playlist   ESC: back");
}

bool PlaylistScreen::HandleEvent(const SDL_Event& event)
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
            m_SelectedIndex = std::min(static_cast<int>(m_Songs.size()), m_SelectedIndex + 1);
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (m_SelectedIndex == 0)
            {
                if (!m_Songs.empty())
                {
                    playback.PlayTracks(m_Songs, 0);
                }
            }
            else if (m_SelectedIndex - 1 < static_cast<int>(m_Songs.size()))
            {
                playback.PlayTracks(m_Songs, m_SelectedIndex - 1);
            }
            return true;
        case SDLK_R:
            if (m_SelectedIndex > 0 &&
                m_SelectedIndex - 1 < static_cast<int>(m_Songs.size()))
            {
                Ctx().playlists->RemoveTrackAt(m_PlaylistId, m_SelectedIndex - 1);
                m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
            }
            return true;
        case SDLK_SPACE:
            playback.Toggle();
            return true;
        default:
            break;
    }
    return false;
}
} // namespace flachead::dap
