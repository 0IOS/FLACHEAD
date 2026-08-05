#include "LibraryScreens.hpp"

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
const Color kAccent   = Color{124, 58, 237, 255};
const Color kFg       = Color{226, 232, 240, 255};
const Color kFgMuted  = Color{148, 163, 184, 255};
const Color kLine     = Color{30, 36, 51, 255};
const Color kRowSel   = Color{28, 35, 51, 255};
const Color kRowBg    = Color{16, 20, 30, 255};
const Color kRowHighlight = Color{34, 211, 238, 255};
constexpr float kListRowH = 44.0f;
constexpr float kListTop = kStatusH + kHeaderH + 6.0f;

const char* kListHints = "UP/DOWN: select   ENTER: play   SPACE: play/pause   ESC: back";

void DrawSongRow(flachead::ui::Canvas& canvas, int width, float y, float rowH,
                 const flachead::models::SongModel& song, int index, bool selected,
                 bool isCurrent)
{
    canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), rowH},
                    selected ? kRowSel : kRowBg);
    if (selected)
    {
        canvas.FillRect(Rect{0.0f, y, 4.0f, rowH}, kAccent);
    }
    canvas.DrawLine(0.0f, y + rowH, static_cast<float>(width), y + rowH, kLine);

    char num[8];
    std::snprintf(num, sizeof(num), "%d.", index);
    canvas.DrawText(Rect{16.0f, y + 12.0f, 60.0f, 20.0f},
                    isCurrent ? "▶" : num,
                    isCurrent ? kRowHighlight : Color{100, 116, 139, 255}, 15.0f);

    std::string title = song.DisplayTitle();
    if (song.favorite)
    {
        title += "  ♥";
    }
    canvas.DrawText(Rect{80.0f, y + 12.0f, static_cast<float>(width) - 340.0f, 20.0f}, title,
                    selected ? Color::White : kFg, 16.0f);
    canvas.DrawText(Rect{static_cast<float>(width) - 260.0f, y + 12.0f, 170.0f, 20.0f},
                    song.DisplayArtist(), kFgMuted, 13.0f);
    canvas.DrawText(Rect{static_cast<float>(width) - 90.0f, y + 12.0f, 70.0f, 20.0f},
                    DapScreen::FormatDuration(song.duration), kFgMuted, 13.0f);
}
} // namespace

// ===========================================================================
// SongListScreen base
// ===========================================================================
SongListScreen::SongListScreen(const AppContext& context, std::string title)
    : DapScreen(context, std::move(title))
{
    Subscribe(flachead::events::Type::QueueChanged);
    Subscribe(flachead::events::Type::TrackChanged);
    Subscribe(flachead::events::Type::PlaybackStopped);
    Subscribe(flachead::events::Type::PlaybackFinished);
}

void SongListScreen::RenderSongs(flachead::ui::Canvas& canvas, int width, int height,
                                 const std::string& hints)
{
    if (m_Songs.empty())
    {
        DrawEmpty(canvas, width, height, "No songs yet — run a scan from Home → Scan");
        DrawFooter(canvas, width, height, hints);
        return;
    }

    m_SelectedIndex = std::clamp(m_SelectedIndex, 0, static_cast<int>(m_Songs.size()) - 1);

    const float maxY = static_cast<float>(height) - kFooterH - 6.0f;
    const int visible = std::max(1, static_cast<int>((maxY - kListTop) / kListRowH));
    const int startRow = std::max(0, m_SelectedIndex - visible / 2);
    const int endRow = std::min(static_cast<int>(m_Songs.size()), startRow + visible);

    const auto& currentPath = Ctx().playback->CurrentTrack().path;
    for (int i = startRow; i < endRow; ++i)
    {
        const float y = kListTop + static_cast<float>(i - startRow) * kListRowH;
        const auto& song = m_Songs[static_cast<std::size_t>(i)];
        DrawSongRow(canvas, width, y, kListRowH, song, i + 1, i == m_SelectedIndex,
                    !currentPath.empty() && song.path == currentPath);
    }

    DrawFooter(canvas, width, height, hints);
}

bool SongListScreen::HandleSongListEvent(const SDL_Event& event)
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
            if (!m_Songs.empty())
            {
                m_SelectedIndex = std::min(static_cast<int>(m_Songs.size()) - 1,
                                           m_SelectedIndex + 1);
            }
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (!m_Songs.empty())
            {
                playback.PlayTracks(m_Songs, m_SelectedIndex);
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

// ===========================================================================
// SongsScreen
// ===========================================================================
SongsScreen::SongsScreen(const AppContext& context)
    : SongListScreen(context, "Library")
{
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void SongsScreen::RefreshData()
{
    m_Songs = Ctx().library->AllSongs();
}

void SongsScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title(), std::to_string(m_Songs.size()) + " song(s)");
    RenderSongs(canvas, width, height, kListHints);
}

bool SongsScreen::HandleEvent(const SDL_Event& event)
{
    return HandleSongListEvent(event);
}

// ===========================================================================
// AlbumsScreen
// ===========================================================================
AlbumsScreen::AlbumsScreen(const AppContext& context)
    : DapScreen(context, "Albums")
{
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void AlbumsScreen::RefreshData()
{
    m_Albums = Ctx().library->Albums();
    m_SelectedIndex = std::clamp(m_SelectedIndex, 0,
                                 std::max(0, static_cast<int>(m_Albums.size()) - 1));
}

void AlbumsScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title(), std::to_string(m_Albums.size()) + " album(s)");

    if (m_Albums.empty())
    {
        DrawEmpty(canvas, width, height, "No albums yet — run a scan from Home → Scan");
        DrawFooter(canvas, width, height, "ESC: back");
        return;
    }

    constexpr int   kCols = 3;
    constexpr float kCardW = 250.0f;
    constexpr float kCardH = 190.0f;
    constexpr float kGapX = 24.0f;
    constexpr float kGapY = 22.0f;
    const float gridW = kCols * kCardW + (kCols - 1) * kGapX;
    const float startX = (static_cast<float>(width) - gridW) * 0.5f;
    const float startY = kStatusH + kHeaderH + 16.0f;

    const int count = static_cast<int>(m_Albums.size());
    for (int idx = 0; idx < count; ++idx)
    {
        const int row = idx / kCols;
        const int col = idx % kCols;
        const float cx = startX + col * (kCardW + kGapX);
        const float cy = startY + row * (kCardH + kGapY);
        const bool selected = idx == m_SelectedIndex;

        const auto& album = m_Albums[static_cast<std::size_t>(idx)];
        canvas.FillRoundedRect(Rect{cx, cy, kCardW, kCardH}, 10.0f,
                               selected ? kRowSel : kRowBg);
        canvas.DrawRoundedRect(Rect{cx, cy, kCardW, kCardH}, 10.0f,
                               selected ? kAccent : kLine);

        const Color art = ArtPlaceholder(album.artPath.empty() ? album.name : album.artPath);
        canvas.FillRoundedRect(Rect{cx + 14.0f, cy + 12.0f, kCardW - 28.0f, kCardW - 28.0f},
                               8.0f, art);
        canvas.DrawTextCentered(Rect{cx + 14.0f, cy + 12.0f, kCardW - 28.0f, kCardW - 28.0f},
                                "♪", Color{20, 24, 36, 255}, 42.0f);

        canvas.DrawText(Rect{cx + 14.0f, cy + kCardW - 12.0f, kCardW - 28.0f, 18.0f},
                        album.DisplayName(), selected ? Color::White : kFg, 15.0f);
        std::string sub = std::to_string(album.trackCount) + " track(s)";
        if (album.year > 0)
        {
            sub += "  ·  " + std::to_string(album.year);
        }
        canvas.DrawText(Rect{cx + 14.0f, cy + kCardW + 8.0f, kCardW - 28.0f, 14.0f}, sub,
                        kFgMuted, 12.0f);
    }

    DrawFooter(canvas, width, height,
               "ARROWS: navigate grid   ENTER: open album   ESC: back");
}

void AlbumsScreen::OpenAlbum(int index)
{
    if (index < 0 || index >= static_cast<int>(m_Albums.size()))
    {
        return;
    }
    const auto& album = m_Albums[static_cast<std::size_t>(index)];
    Ctx().pending.album = album.name;
    Ctx().pending.albumArtist = album.artist;
    Ctx().navigate("album");
}

bool AlbumsScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
    {
        return false;
    }

    const int count = static_cast<int>(m_Albums.size());
    if (count == 0)
    {
        if (event.key.key == SDLK_ESCAPE)
        {
            Ctx().goBack();
            return true;
        }
        return false;
    }

    constexpr int kCols = 3;
    switch (event.key.key)
    {
        case SDLK_ESCAPE:
            Ctx().goBack();
            return true;
        case SDLK_LEFT:
            m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
            return true;
        case SDLK_RIGHT:
            m_SelectedIndex = std::min(count - 1, m_SelectedIndex + 1);
            return true;
        case SDLK_UP:
            m_SelectedIndex = std::max(0, m_SelectedIndex - kCols);
            return true;
        case SDLK_DOWN:
            m_SelectedIndex = std::min(count - 1, m_SelectedIndex + kCols);
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            OpenAlbum(m_SelectedIndex);
            return true;
        default:
            break;
    }
    return false;
}

// ===========================================================================
// AlbumScreen
// ===========================================================================
AlbumScreen::AlbumScreen(const AppContext& context)
    : SongListScreen(context, "Album")
{
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void AlbumScreen::RefreshData()
{
    m_AlbumName = Ctx().pending.album;
    m_AlbumArtist = Ctx().pending.albumArtist;
    m_Songs = Ctx().library->AlbumTracks(m_AlbumName, m_AlbumArtist);
}

void AlbumScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);

    std::string subtitle = m_AlbumName;
    if (m_AlbumArtist.empty())
    {
        subtitle += "  ·  " + std::to_string(m_Songs.size()) + " track(s)";
    }
    DrawHeader(canvas, width, Title(), subtitle);

    // "Play album" action row.
    const float py = kStatusH + kHeaderH + 6.0f;
    canvas.FillRect(Rect{0.0f, py, static_cast<float>(width), kListRowH},
                    m_SelectedIndex == 0 ? kRowSel : kRowBg);
    if (m_SelectedIndex == 0)
    {
        canvas.FillRect(Rect{0.0f, py, 4.0f, kListRowH}, kAccent);
    }
    canvas.DrawLine(0.0f, py + kListRowH, static_cast<float>(width), py + kListRowH, kLine);
    canvas.DrawText(Rect{22.0f, py + 12.0f, 300.0f, 20.0f}, "▶ Play album",
                    m_SelectedIndex == 0 ? Color::White : Color{52, 211, 153, 255}, 16.0f);
    if (!m_AlbumArtist.empty())
    {
        canvas.DrawText(Rect{static_cast<float>(width) - 240.0f, py + 12.0f, 220.0f, 20.0f},
                        m_AlbumArtist, kFgMuted, 13.0f);
    }

    if (m_Songs.empty())
    {
        DrawEmpty(canvas, width, height, "No tracks in this album");
        DrawFooter(canvas, width, height, "ESC: back");
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
        DrawSongRow(canvas, width, y, kListRowH, song, i + 1,
                    i == sel, !currentPath.empty() && song.path == currentPath);
    }

    DrawFooter(canvas, width, height, kListHints);
}

bool AlbumScreen::HandleEvent(const SDL_Event& event)
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
        case SDLK_SPACE:
            playback.Toggle();
            return true;
        default:
            break;
    }
    return false;
}

// ===========================================================================
// ArtistsScreen
// ===========================================================================
ArtistsScreen::ArtistsScreen(const AppContext& context)
    : DapScreen(context, "Artists")
{
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void ArtistsScreen::RefreshData()
{
    m_Artists.clear();
    const auto albums = Ctx().library->Albums();
    const auto artists = Ctx().library->Artists();

    struct ArtistAgg
    {
        std::string name;
        int albums{0};
        int tracks{0};
    };
    std::vector<ArtistAgg> agg;
    for (const auto& a : artists)
    {
        agg.push_back({a.name, 0, a.trackCount});
    }
    for (const auto& al : albums)
    {
        auto it = std::find_if(agg.begin(), agg.end(),
                               [&](const ArtistAgg& x) { return x.name == al.artist; });
        if (it != agg.end())
        {
            ++it->albums;
        }
    }

    for (const auto& a : agg)
    {
        m_Artists.push_back({a.name, a.albums, a.tracks});
    }
    m_SelectedIndex = std::clamp(m_SelectedIndex, 0,
                                 std::max(0, static_cast<int>(m_Artists.size()) - 1));
}

void ArtistsScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title(), std::to_string(m_Artists.size()) + " artist(s)");

    if (m_Artists.empty())
    {
        DrawEmpty(canvas, width, height, "No artists yet — run a scan from Home → Scan");
        DrawFooter(canvas, width, height, "ESC: back");
        return;
    }

    const float maxY = static_cast<float>(height) - kFooterH - 6.0f;
    const int visible = std::max(1, static_cast<int>((maxY - kListTop) / kListRowH));
    const int startRow = std::max(0, m_SelectedIndex - visible / 2);
    const int endRow = std::min(static_cast<int>(m_Artists.size()), startRow + visible);

    for (int i = startRow; i < endRow; ++i)
    {
        const float y = kListTop + static_cast<float>(i - startRow) * kListRowH;
        const auto& a = m_Artists[static_cast<std::size_t>(i)];
        const bool selected = i == m_SelectedIndex;
        canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), kListRowH},
                        selected ? kRowSel : kRowBg);
        if (selected)
        {
            canvas.FillRect(Rect{0.0f, y, 4.0f, kListRowH}, kAccent);
        }
        canvas.DrawLine(0.0f, y + kListRowH, static_cast<float>(width), y + kListRowH, kLine);

        canvas.DrawText(Rect{22.0f, y + 12.0f, static_cast<float>(width) - 260.0f, 20.0f},
                        a.name, selected ? Color::White : kFg, 16.0f);
        std::string meta = std::to_string(a.albums) + " album(s) · " +
                           std::to_string(a.tracks) + " track(s)";
        canvas.DrawText(Rect{static_cast<float>(width) - 240.0f, y + 12.0f, 220.0f, 20.0f},
                        meta, kFgMuted, 13.0f);
    }

    DrawFooter(canvas, width, height, "UP/DOWN: select   ENTER: artist tracks   ESC: back");
}

bool ArtistsScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
    {
        return false;
    }

    switch (event.key.key)
    {
        case SDLK_ESCAPE:
            Ctx().goBack();
            return true;
        case SDLK_UP:
            m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
            return true;
        case SDLK_DOWN:
            m_SelectedIndex = std::min(static_cast<int>(m_Artists.size()) - 1,
                                       m_SelectedIndex + 1);
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (!m_Artists.empty())
            {
                Ctx().pending.artist = m_Artists[static_cast<std::size_t>(m_SelectedIndex)].name;
                Ctx().navigate("artist");
            }
            return true;
        default:
            break;
    }
    return false;
}

// ===========================================================================
// ArtistScreen
// ===========================================================================
ArtistScreen::ArtistScreen(const AppContext& context)
    : SongListScreen(context, "Artist")
{
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void ArtistScreen::RefreshData()
{
    m_ArtistName = Ctx().pending.artist;
    m_Songs.clear();
    const auto all = Ctx().library->AllSongs();
    for (const auto& song : all)
    {
        if (song.artist == m_ArtistName || song.albumArtist == m_ArtistName)
        {
            m_Songs.push_back(song);
        }
    }
}

void ArtistScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);

    std::string subtitle = m_ArtistName;
    if (!subtitle.empty())
    {
        subtitle += "  ·  " + std::to_string(m_Songs.size()) + " track(s)";
    }
    DrawHeader(canvas, width, Title(), subtitle);
    RenderSongs(canvas, width, height, kListHints);
}

bool ArtistScreen::HandleEvent(const SDL_Event& event)
{
    return HandleSongListEvent(event);
}

// ===========================================================================
// SearchScreen
// ===========================================================================
SearchScreen::SearchScreen(const AppContext& context)
    : DapScreen(context, "Search")
{
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void SearchScreen::RunQuery()
{
    m_Results = Ctx().library->Search(m_Query);
    m_SelectedIndex = std::clamp(m_SelectedIndex, 0,
                                 std::max(0, static_cast<int>(m_Results.size()) - 1));
}

void SearchScreen::RefreshData()
{
    RunQuery();
}

void SearchScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title(),
               std::to_string(m_Results.size()) + " result(s)");

    // Query box.
    constexpr float boxW = 560.0f;
    const float bx = (static_cast<float>(width) - boxW) * 0.5f;
    const float by = kStatusH + kHeaderH + 14.0f;
    canvas.FillRoundedRect(Rect{bx, by, boxW, 42.0f}, 8.0f, kRowBg);
    canvas.DrawRoundedRect(Rect{bx, by, boxW, 42.0f}, 8.0f, kAccent);
    canvas.DrawText(Rect{bx + 16.0f, by + 10.0f, boxW - 32.0f, 22.0f},
                    m_Query.empty() ? "Type to search songs, artists, albums…" : m_Query,
                    m_Query.empty() ? kFgMuted : Color::White, 16.0f);
    if (!m_Query.empty())
    {
        canvas.DrawText(Rect{bx + 16.0f + static_cast<float>(m_Query.size()) * 9.0f,
                             by + 10.0f, 10.0f, 22.0f},
                        "|", kAccent, 16.0f);
    }

    if (m_Query.empty())
    {
        DrawEmpty(canvas, width, height, "Search your library");
        DrawFooter(canvas, width, height,
                   "TYPE: query   UP/DOWN: select   ENTER: play   ESC: back");
        return;
    }

    const float maxY = static_cast<float>(height) - kFooterH - 6.0f;
    const float top = by + 42.0f + 10.0f;
    const int visible = std::max(1, static_cast<int>((maxY - top) / kListRowH));
    const int startRow = std::max(0, m_SelectedIndex - visible / 2);
    const int endRow = std::min(static_cast<int>(m_Results.size()), startRow + visible);

    const auto& currentPath = Ctx().playback->CurrentTrack().path;
    for (int i = startRow; i < endRow; ++i)
    {
        const float y = top + static_cast<float>(i - startRow) * kListRowH;
        const auto& song = m_Results[static_cast<std::size_t>(i)];
        DrawSongRow(canvas, width, y, kListRowH, song, i + 1, i == m_SelectedIndex,
                    !currentPath.empty() && song.path == currentPath);
    }

    DrawFooter(canvas, width, height,
               "TYPE: query   UP/DOWN: select   ENTER: play   ESC: back");
}

bool SearchScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
    {
        return false;
    }

    auto& playback = *Ctx().playback;
    const SDL_Keycode key = event.key.key;
    switch (key)
    {
        case SDLK_ESCAPE:
            Ctx().goBack();
            return true;
        case SDLK_BACKSPACE:
            if (!m_Query.empty())
            {
                m_Query.pop_back();
                RunQuery();
            }
            return true;
        case SDLK_UP:
            m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
            return true;
        case SDLK_DOWN:
            if (!m_Results.empty())
            {
                m_SelectedIndex = std::min(static_cast<int>(m_Results.size()) - 1,
                                           m_SelectedIndex + 1);
            }
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (!m_Results.empty())
            {
                playback.PlayTracks(m_Results, m_SelectedIndex);
            }
            return true;
        case SDLK_SPACE:
            m_Query += ' ';
            RunQuery();
            return true;
        default:
            if (key >= ' ' && key <= '~')
            {
                m_Query += static_cast<char>(key);
                RunQuery();
                return true;
            }
            break;
    }
    return false;
}

// ===========================================================================
// FavoritesScreen
// ===========================================================================
FavoritesScreen::FavoritesScreen(const AppContext& context)
    : SongListScreen(context, "Favorites")
{
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void FavoritesScreen::RefreshData()
{
    m_Songs = Ctx().library->Favorites();
}

void FavoritesScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title(), std::to_string(m_Songs.size()) + " song(s)");
    RenderSongs(canvas, width, height, kListHints);
}

bool FavoritesScreen::HandleEvent(const SDL_Event& event)
{
    return HandleSongListEvent(event);
}

// ===========================================================================
// RecentScreen
// ===========================================================================
RecentScreen::RecentScreen(const AppContext& context)
    : SongListScreen(context, "Recently Added")
{
    Subscribe(flachead::events::Type::LibraryUpdated);
}

void RecentScreen::RefreshData()
{
    m_Songs = Ctx().library->RecentlyAdded(50);
}

void RecentScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title(), std::to_string(m_Songs.size()) + " song(s)");
    RenderSongs(canvas, width, height, kListHints);
}

bool RecentScreen::HandleEvent(const SDL_Event& event)
{
    return HandleSongListEvent(event);
}
} // namespace flachead::dap
