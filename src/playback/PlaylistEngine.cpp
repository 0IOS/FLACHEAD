#include "PlaylistEngine.hpp"

#include "../core/Logger.hpp"

#include <sqlite3.h>

#include <ctime>

namespace flachead::playback
{
using flachead::database::Statement;
using flachead::database::Transaction;

PlaylistEngine::PlaylistEngine(flachead::database::Database& db,
                               flachead::events::EventBus& eventBus)
    : m_Db(db),
      m_EventBus(eventBus)
{
}

std::vector<PlaylistEngine::PlaylistInfo> PlaylistEngine::ListPlaylists() const
{
    std::vector<PlaylistInfo> result;

    Statement stmt = m_Db.Prepare(
        "SELECT p.id, p.name, COUNT(t.song_id) "
        "FROM playlists p LEFT JOIN playlist_tracks t ON t.playlist_id = p.id "
        "GROUP BY p.id ORDER BY p.name COLLATE NOCASE;");
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(PlaylistInfo{
            stmt.ColumnInt64(0),
            stmt.ColumnText(1),
            stmt.ColumnInt(2),
        });
    }
    return result;
}

std::string PlaylistEngine::PlaylistName(std::int64_t id) const
{
    Statement stmt = m_Db.Prepare("SELECT name FROM playlists WHERE id = ?;");
    stmt.Bind(1, id);
    if (stmt.Step() == SQLITE_ROW)
    {
        return stmt.ColumnText(0);
    }
    return {};
}

std::int64_t PlaylistEngine::CreatePlaylist(std::string_view name)
{
    Transaction txn{m_Db.Handle()};

    // Return the existing playlist when the name already exists.
    Statement lookup = m_Db.Prepare("SELECT id FROM playlists WHERE name = ?;");
    lookup.Bind(1, name);
    if (lookup.Step() == SQLITE_ROW)
    {
        txn.Commit();
        return lookup.ColumnInt64(0);
    }

    Statement insert = m_Db.Prepare("INSERT INTO playlists (name, created) VALUES (?, ?);");
    insert.Bind(1, name);
    insert.Bind(2, static_cast<std::int64_t>(time(nullptr)));
    if (insert.Step() != SQLITE_DONE)
    {
        txn.Commit();
        return 0;
    }

    const std::int64_t id = m_Db.LastInsertRowId();
    txn.Commit();

    m_EventBus.Publish(flachead::events::Type::PlaylistChanged);
    return id;
}

bool PlaylistEngine::RenamePlaylist(std::int64_t id, std::string_view name)
{
    Statement stmt = m_Db.Prepare("UPDATE playlists SET name = ? WHERE id = ?;");
    stmt.Bind(1, name);
    stmt.Bind(2, id);
    const bool ok = stmt.Step() == SQLITE_DONE;
    if (ok)
    {
        m_EventBus.Publish(flachead::events::Type::PlaylistChanged);
    }
    return ok;
}

bool PlaylistEngine::DeletePlaylist(std::int64_t id)
{
    Statement stmt = m_Db.Prepare("DELETE FROM playlists WHERE id = ?;");
    stmt.Bind(1, id);
    const bool ok = stmt.Step() == SQLITE_DONE;
    if (ok)
    {
        m_EventBus.Publish(flachead::events::Type::PlaylistChanged);
    }
    return ok;
}

std::vector<std::int64_t> PlaylistEngine::TrackIds(std::int64_t playlistId) const
{
    std::vector<std::int64_t> result;

    Statement stmt = m_Db.Prepare(
        "SELECT song_id FROM playlist_tracks "
        "WHERE playlist_id = ? ORDER BY position;");
    stmt.Bind(1, playlistId);
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(stmt.ColumnInt64(0));
    }
    return result;
}

bool PlaylistEngine::AddTrack(std::int64_t playlistId, std::int64_t songId)
{
    Statement count = m_Db.Prepare("SELECT COALESCE(MAX(position), -1) FROM playlist_tracks WHERE playlist_id = ?;");
    count.Bind(1, playlistId);
    int next = 0;
    if (count.Step() == SQLITE_ROW)
    {
        next = count.ColumnInt(0) + 1;
    }

    Statement insert = m_Db.Prepare(
        "INSERT INTO playlist_tracks (playlist_id, position, song_id) VALUES (?, ?, ?);");
    insert.Bind(1, playlistId);
    insert.Bind(2, next);
    insert.Bind(3, songId);
    const bool ok = insert.Step() == SQLITE_DONE;
    if (ok)
    {
        m_EventBus.Publish(flachead::events::Type::PlaylistChanged);
    }
    return ok;
}

bool PlaylistEngine::RemoveTrackAt(std::int64_t playlistId, int position)
{
    Statement remove = m_Db.Prepare(
        "DELETE FROM playlist_tracks WHERE playlist_id = ? AND position = ?;");
    remove.Bind(1, playlistId);
    remove.Bind(2, position);
    const bool ok = remove.Step() == SQLITE_DONE;

    // Compact positions so ordering stays dense.
    Statement renumber = m_Db.Prepare(
        "UPDATE playlist_tracks SET position = position - 1 "
        "WHERE playlist_id = ? AND position > ?;");
    renumber.Bind(1, playlistId);
    renumber.Bind(2, position);
    renumber.Step();

    if (ok)
    {
        m_EventBus.Publish(flachead::events::Type::PlaylistChanged);
    }
    return ok;
}

bool PlaylistEngine::MoveTrack(std::int64_t playlistId, int fromPosition, int toPosition)
{
    if (fromPosition == toPosition)
    {
        return true;
    }

    std::vector<std::int64_t> ids = TrackIds(playlistId);
    if (fromPosition < 0 || fromPosition >= static_cast<int>(ids.size()) ||
        toPosition < 0 || toPosition >= static_cast<int>(ids.size()))
    {
        return false;
    }

    // Reorder in memory, then rewrite every position in one transaction.
    const std::int64_t moved = ids[static_cast<std::size_t>(fromPosition)];
    ids.erase(ids.begin() + fromPosition);
    ids.insert(ids.begin() + toPosition, moved);

    Transaction txn{m_Db.Handle()};
    Statement clear = m_Db.Prepare("DELETE FROM playlist_tracks WHERE playlist_id = ?;");
    clear.Bind(1, playlistId);
    clear.Step();

    Statement insert = m_Db.Prepare(
        "INSERT INTO playlist_tracks (playlist_id, position, song_id) VALUES (?, ?, ?);");
    for (std::size_t i = 0; i < ids.size(); ++i)
    {
        insert.Bind(1, playlistId);
        insert.Bind(2, static_cast<int>(i));
        insert.Bind(3, ids[i]);
        insert.Step();
        insert.Reset();
    }

    txn.Commit();
    m_EventBus.Publish(flachead::events::Type::PlaylistChanged);
    return true;
}

bool PlaylistEngine::ClearPlaylist(std::int64_t playlistId)
{
    Statement stmt = m_Db.Prepare("DELETE FROM playlist_tracks WHERE playlist_id = ?;");
    stmt.Bind(1, playlistId);
    const bool ok = stmt.Step() == SQLITE_DONE;
    if (ok)
    {
        m_EventBus.Publish(flachead::events::Type::PlaylistChanged);
    }
    return ok;
}
} // namespace flachead::playback
