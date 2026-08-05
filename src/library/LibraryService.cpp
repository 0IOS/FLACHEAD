#include "LibraryService.hpp"

#include "../core/Logger.hpp"

#include <sqlite3.h>

#include <algorithm>
#include <chrono>

namespace flachead::library
{
using flachead::database::Statement;
using flachead::events::EventBus;
using flachead::models::AlbumModel;
using flachead::models::ArtistModel;
using flachead::models::SongModel;

namespace
{
#define kSongColumns "id, path, title, artist, album, album_artist, genre, folder, codec, " \
                     "track_no, disc_no, year, channels, sample_rate, bitrate, duration, " \
                     "file_size, file_mtime, date_added, date_played, play_count, favorite, " \
                     "has_art, art_path"

std::string LikePattern(const std::string& query)
{
    std::string pattern = "%";
    for (const char c : query)
    {
        if (c == '%' || c == '_')
        {
            pattern.push_back('\\');
        }
        pattern.push_back(c);
    }
    pattern.push_back('%');
    return pattern;
}
} // namespace

LibraryService::LibraryService(flachead::database::Database& db, EventBus& eventBus)
    : m_Db(db),
      m_EventBus(eventBus),
      m_Scanner(db, eventBus)
{
}

SongModel LibraryService::RowToSong(Statement& stmt) const
{
    SongModel song;
    song.id = stmt.ColumnInt64(0);
    song.path = stmt.ColumnText(1);
    song.title = stmt.ColumnText(2);
    song.artist = stmt.ColumnText(3);
    song.album = stmt.ColumnText(4);
    song.albumArtist = stmt.ColumnText(5);
    song.genre = stmt.ColumnText(6);
    song.folder = stmt.ColumnText(7);
    song.codec = stmt.ColumnText(8);
    song.trackNo = stmt.ColumnInt(9);
    song.discNo = stmt.ColumnInt(10);
    song.year = stmt.ColumnInt(11);
    song.channels = stmt.ColumnInt(12);
    song.sampleRate = stmt.ColumnInt(13);
    song.bitrate = stmt.ColumnInt(14);
    song.duration = stmt.ColumnDouble(15);
    song.fileSize = stmt.ColumnInt64(16);
    song.fileMtime = stmt.ColumnInt64(17);
    song.dateAdded = stmt.ColumnInt64(18);
    song.datePlayed = stmt.ColumnInt64(19);
    song.playCount = stmt.ColumnInt(20);
    song.favorite = stmt.ColumnInt(21) != 0;
    song.hasEmbeddedArt = stmt.ColumnInt(22) != 0;
    song.artPath = stmt.ColumnText(23);
    return song;
}

bool LibraryService::StartScan(const std::vector<std::string>& roots)
{
    return m_Scanner.StartScan(roots);
}

int LibraryService::SongCount() const
{
    Statement stmt = m_Db.Prepare("SELECT COUNT(*) FROM songs;");
    if (stmt.Step() == SQLITE_ROW)
    {
        return stmt.ColumnInt(0);
    }
    return 0;
}

std::vector<SongModel> LibraryService::AllSongs() const
{
    std::vector<SongModel> result;
    Statement stmt = m_Db.Prepare(
        "SELECT " kSongColumns " FROM songs "
        "ORDER BY artist COLLATE NOCASE, album COLLATE NOCASE, disc_no, track_no;");
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(RowToSong(stmt));
    }
    return result;
}

std::vector<SongModel> LibraryService::SongsInFolder(const std::string& folder) const
{
    std::vector<SongModel> result;
    Statement stmt = m_Db.Prepare(
        "SELECT " kSongColumns " FROM songs WHERE folder = ? "
        "ORDER BY disc_no, track_no;");
    stmt.Bind(1, folder);
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(RowToSong(stmt));
    }
    return result;
}

std::vector<SongModel> LibraryService::Search(const std::string& query) const
{
    std::vector<SongModel> result;
    const std::string pattern = LikePattern(query);
    Statement stmt = m_Db.Prepare(
        "SELECT " kSongColumns " FROM songs "
        "WHERE title LIKE ? ESCAPE '\\' OR artist LIKE ? ESCAPE '\\' OR album LIKE ? ESCAPE '\\' "
        "ORDER BY artist COLLATE NOCASE, album COLLATE NOCASE, track_no LIMIT 200;");
    stmt.Bind(1, pattern);
    stmt.Bind(2, pattern);
    stmt.Bind(3, pattern);
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(RowToSong(stmt));
    }
    return result;
}

std::vector<SongModel> LibraryService::Recent(int limit) const
{
    std::vector<SongModel> result;
    Statement stmt = m_Db.Prepare(
        "SELECT " kSongColumns " FROM songs "
        "WHERE date_played > 0 ORDER BY date_played DESC LIMIT ?;");
    stmt.Bind(1, limit);
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(RowToSong(stmt));
    }
    return result;
}

std::vector<SongModel> LibraryService::RecentlyAdded(int limit) const
{
    std::vector<SongModel> result;
    Statement stmt = m_Db.Prepare(
        "SELECT " kSongColumns " FROM songs "
        "ORDER BY date_added DESC, id DESC LIMIT ?;");
    stmt.Bind(1, limit);
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(RowToSong(stmt));
    }
    return result;
}

std::vector<SongModel> LibraryService::Favorites() const
{
    std::vector<SongModel> result;
    Statement stmt = m_Db.Prepare(
        "SELECT " kSongColumns " FROM songs "
        "WHERE favorite = 1 ORDER BY artist COLLATE NOCASE, album COLLATE NOCASE, track_no;");
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(RowToSong(stmt));
    }
    return result;
}

std::vector<SongModel> LibraryService::DuplicateTitles() const
{
    std::vector<SongModel> result;
    // Same title+artist on different paths.
    Statement stmt = m_Db.Prepare(
        "SELECT " kSongColumns " FROM songs WHERE title <> '' AND id IN "
        "(SELECT id FROM songs GROUP BY title, artist, path HAVING COUNT(*) > 1) "
        "ORDER BY title COLLATE NOCASE;");
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(RowToSong(stmt));
    }
    return result;
}

std::vector<AlbumModel> LibraryService::Albums() const
{
    std::vector<AlbumModel> result;
    Statement stmt = m_Db.Prepare(
        "SELECT album, album_artist, art_path, MIN(year) AS year, COUNT(*) AS track_count "
        "FROM songs WHERE album <> '' GROUP BY album, album_artist "
        "ORDER BY album COLLATE NOCASE;");
    while (stmt.Step() == SQLITE_ROW)
    {
        AlbumModel album;
        album.name = stmt.ColumnText(0);
        album.artist = stmt.ColumnText(1);
        album.artPath = stmt.ColumnText(2);
        album.year = stmt.ColumnInt(3);
        album.trackCount = stmt.ColumnInt(4);
        result.push_back(std::move(album));
    }
    return result;
}

std::vector<ArtistModel> LibraryService::Artists() const
{
    std::vector<ArtistModel> result;
    Statement stmt = m_Db.Prepare(
        "SELECT artist, COUNT(*) FROM songs WHERE artist <> '' "
        "GROUP BY artist COLLATE NOCASE ORDER BY artist COLLATE NOCASE;");
    while (stmt.Step() == SQLITE_ROW)
    {
        ArtistModel artist;
        artist.name = stmt.ColumnText(0);
        artist.trackCount = stmt.ColumnInt(1);
        result.push_back(std::move(artist));
    }
    return result;
}

std::vector<SongModel> LibraryService::AlbumTracks(const std::string& album,
                                                   const std::string& artist) const
{
    std::vector<SongModel> result;
    Statement stmt = m_Db.Prepare(
        "SELECT " kSongColumns " FROM songs "
        "WHERE album = ? AND (album_artist = ? OR artist = ?) "
        "ORDER BY disc_no, track_no;");
    stmt.Bind(1, album);
    stmt.Bind(2, artist);
    stmt.Bind(3, artist);
    while (stmt.Step() == SQLITE_ROW)
    {
        result.push_back(RowToSong(stmt));
    }
    return result;
}

SongModel LibraryService::FindByPath(const std::string& path) const
{
    Statement stmt = m_Db.Prepare("SELECT " kSongColumns " FROM songs WHERE path = ?;");
    stmt.Bind(1, path);
    if (stmt.Step() == SQLITE_ROW)
    {
        return RowToSong(stmt);
    }
    return {};
}

SongModel LibraryService::FindById(std::int64_t id) const
{
    Statement stmt = m_Db.Prepare("SELECT " kSongColumns " FROM songs WHERE id = ?;");
    stmt.Bind(1, id);
    if (stmt.Step() == SQLITE_ROW)
    {
        return RowToSong(stmt);
    }
    return {};
}

bool LibraryService::ToggleFavorite(std::int64_t id)
{
    Statement stmt = m_Db.Prepare(
        "UPDATE songs SET favorite = 1 - favorite WHERE id = ?;");
    stmt.Bind(1, id);
    if (stmt.Step() != SQLITE_DONE)
    {
        return false;
    }
    m_EventBus.Publish(flachead::events::Type::LibraryUpdated);
    return true;
}

void LibraryService::MarkPlayed(const SongModel& song)
{
    if (song.id <= 0)
    {
        return;
    }
    Statement stmt = m_Db.Prepare(
        "UPDATE songs SET play_count = play_count + 1, date_played = ? WHERE id = ?;");
    stmt.Bind(1, static_cast<std::int64_t>(
                    std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count()));
    stmt.Bind(2, song.id);
    stmt.Step();

    Statement hist = m_Db.Prepare(
        "INSERT INTO play_history (song_id, played_at) VALUES (?, ?);");
    hist.Bind(1, song.id);
    hist.Bind(2, static_cast<std::int64_t>(
                    std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count()));
    hist.Step();
}
} // namespace flachead::library
