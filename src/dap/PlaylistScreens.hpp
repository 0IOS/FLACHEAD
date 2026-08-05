#pragma once

#include "../models/SongModel.hpp"
#include "../playback/PlaylistEngine.hpp"
#include "DapScreen.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace flachead::dap
{
// Playlist list + management. ENTER opens a playlist, ENTER on the top "new
// playlist" row creates one, D deletes the selected playlist.
class PlaylistsScreen : public DapScreen
{
public:
    explicit PlaylistsScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::vector<flachead::playback::PlaylistEngine::PlaylistInfo> m_Playlists;
    int m_SelectedIndex{0};
};

// Tracks of a single playlist, with "Play playlist" and per-track remove.
// R removes the selected song from the playlist.
class PlaylistScreen : public DapScreen
{
public:
    explicit PlaylistScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::int64_t m_PlaylistId{0};
    std::string m_PlaylistName;
    std::vector<flachead::models::SongModel> m_Songs;
    int m_SelectedIndex{0};
};
} // namespace flachead::dap
