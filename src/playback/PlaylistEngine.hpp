#pragma once

#include "../database/Database.hpp"
#include "../events/EventBus.hpp"
#include "../models/PlaylistModel.hpp"

#include <cstdint>
#include <vector>

namespace flachead::playback
{
// Playlist CRUD backed by the songs database. Emits PlaylistChanged on every
// mutation so screens refresh. Not thread safe; call from the main thread.
class PlaylistEngine
{
public:
    PlaylistEngine(flachead::database::Database& db, flachead::events::EventBus& eventBus);

    struct PlaylistInfo
    {
        std::int64_t id{0};
        std::string name;
        int trackCount{0};
    };

    std::vector<PlaylistInfo> ListPlaylists() const;
    std::string PlaylistName(std::int64_t id) const;

    std::int64_t CreatePlaylist(std::string_view name);
    bool RenamePlaylist(std::int64_t id, std::string_view name);
    bool DeletePlaylist(std::int64_t id);

    // Track ids (song.id) in playlist order.
    std::vector<std::int64_t> TrackIds(std::int64_t playlistId) const;

    bool AddTrack(std::int64_t playlistId, std::int64_t songId);
    bool RemoveTrackAt(std::int64_t playlistId, int position);
    bool MoveTrack(std::int64_t playlistId, int fromPosition, int toPosition);
    bool ClearPlaylist(std::int64_t playlistId);

private:
    flachead::database::Database& m_Db;
    flachead::events::EventBus& m_EventBus;
};
} // namespace flachead::playback
