#pragma once

#include "../models/AlbumModel.hpp"
#include "../models/SongModel.hpp"
#include "DapScreen.hpp"

#include <string>
#include <vector>

namespace flachead::dap
{
// Shared base for screens that present a scrollable, playable song list with
// keyboard selection. Derived screens fill m_Songs in RefreshData() and get
// list rendering + UP/DOWN/ENTER handling for free. ENTER plays the selected
// track, keeping the whole list as the playback queue.
class SongListScreen : public DapScreen
{
public:
    SongListScreen(const AppContext& context, std::string title);

protected:
    void RenderSongs(flachead::ui::Canvas& canvas, int width, int height,
                     const std::string& hints);
    // Returns true when the event was consumed by the list navigation.
    bool HandleSongListEvent(const SDL_Event& event);

    std::vector<flachead::models::SongModel> m_Songs;
    int m_SelectedIndex{0};
};

// All songs in the library.
class SongsScreen : public SongListScreen
{
public:
    explicit SongsScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;
};

// Album cover grid. ENTER opens the album detail screen.
class AlbumsScreen : public DapScreen
{
public:
    explicit AlbumsScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    void OpenAlbum(int index);

    std::vector<flachead::models::AlbumModel> m_Albums;
    int m_SelectedIndex{0};
};

// Tracks of one album, with a "Play album" action.
class AlbumScreen : public SongListScreen
{
public:
    explicit AlbumScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::string m_AlbumName;
    std::string m_AlbumArtist;
};

// Artist list. ENTER opens the artist detail screen.
class ArtistsScreen : public DapScreen
{
public:
    explicit ArtistsScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    struct ArtistRow
    {
        std::string name;
        int albums{0};
        int tracks{0};
    };

    std::vector<ArtistRow> m_Artists;
    int m_SelectedIndex{0};
};

// Songs by a single artist (destination of ArtistsScreen).
class ArtistScreen : public SongListScreen
{
public:
    explicit ArtistScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::string m_ArtistName;
};

// Text search over the library.
class SearchScreen : public DapScreen
{
public:
    explicit SearchScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    void RunQuery();

    std::string m_Query;
    std::vector<flachead::models::SongModel> m_Results;
    int m_SelectedIndex{0};
};

// Favorite songs.
class FavoritesScreen : public SongListScreen
{
public:
    explicit FavoritesScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;
};

// Recently added songs.
class RecentScreen : public SongListScreen
{
public:
    explicit RecentScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;
};
} // namespace flachead::dap
