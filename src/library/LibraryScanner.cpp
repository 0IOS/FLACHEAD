#include "LibraryScanner.hpp"

#include "../core/Logger.hpp"
#include "../core/PathUtils.hpp"
#include "../library/AlbumArtExtractor.hpp"
#include "../metadata/FFprobeExtractor.hpp"
#include "../metadata/Metadata.hpp"
#include "../models/SongModel.hpp"

#include <sqlite3.h>
#include <sys/stat.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

namespace flachead::library
{
using flachead::database::Statement;
using flachead::events::Event;
using flachead::events::EventBus;
using flachead::events::Type;
using flachead::metadata::FFprobeExtractor;
using flachead::metadata::TrackMetadata;
using flachead::models::SongModel;

namespace
{
constexpr int kProgressEvery = 25;

struct FileStat
{
    int64_t mtime{0};
    int64_t size{0};
};

bool StatFile(const std::string& path, FileStat& out)
{
    struct stat st {};
    if (::stat(path.c_str(), &st) != 0)
    {
        return false;
    }
    out.mtime = static_cast<int64_t>(st.st_mtime);
    out.size = static_cast<int64_t>(st.st_size);
    return true;
}

bool IsUnderRoot(const std::string& path, const std::string& root)
{
    if (path.compare(0, root.size(), root) != 0)
    {
        return false;
    }
    return path.size() == root.size() || root.back() == '/' || path[root.size()] == '/';
}

void MarkAlive(std::vector<int64_t>& staleIds, int64_t id)
{
    staleIds.erase(std::remove(staleIds.begin(), staleIds.end(), id), staleIds.end());
}

int64_t NowSeconds()
{
    return static_cast<int64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());
}
} // namespace

LibraryScanner::LibraryScanner(flachead::database::Database& db, EventBus& eventBus)
    : m_Db(db),
      m_EventBus(eventBus)
{
}

LibraryScanner::~LibraryScanner()
{
    Cancel();
    Wait();
}

bool LibraryScanner::StartScan(const std::vector<std::string>& roots)
{
    if (m_Scanning.load())
    {
        return false;
    }

    std::vector<std::string> valid;
    for (const auto& root : roots)
    {
        if (!root.empty())
        {
            valid.push_back(root);
        }
    }
    if (valid.empty())
    {
        return false;
    }

    m_Cancel = false;
    m_Scanning = true;
    m_Thread = std::thread(&LibraryScanner::ScanWorker, this, std::move(valid));
    return true;
}

void LibraryScanner::Cancel()
{
    m_Cancel = true;
}

void LibraryScanner::Wait()
{
    if (m_Thread.joinable())
    {
        m_Thread.join();
    }
}

void LibraryScanner::ScanWorker(const std::vector<std::string>& roots)
{
    m_EventBus.Publish(Type::LibraryScanStarted);

    std::vector<std::string> audioFiles;
    for (const auto& root : roots)
    {
        std::vector<std::string> files;
        flachead::core::path::CollectFiles(root, files);
        for (const auto& file : files)
        {
            if (flachead::core::path::IsAudioFile(file))
            {
                audioFiles.push_back(file);
            }
        }
    }
    std::sort(audioFiles.begin(), audioFiles.end());
    const std::size_t total = audioFiles.size();
    m_LastCount = static_cast<int>(total);

    // Every song under the scan roots starts in the prune set; files seen on
    // disk are removed from it, and what remains was deleted.
    std::vector<int64_t> staleIds;
    {
        Statement stmt = m_Db.Prepare("SELECT id, path FROM songs;");
        while (stmt.Step() == SQLITE_ROW)
        {
            const std::string path = stmt.ColumnText(1);
            for (const auto& root : roots)
            {
                if (IsUnderRoot(path, root))
                {
                    staleIds.push_back(stmt.ColumnInt64(0));
                    break;
                }
            }
        }
    }

    int processed = 0;
    for (const auto& file : audioFiles)
    {
        if (m_Cancel.load())
        {
            break;
        }
        ScanFile(file, staleIds);
        ++processed;

        if (processed % kProgressEvery == 0)
        {
            Event progress;
            progress.type = Type::LibraryScanProgress;
            progress.intValue = static_cast<int64_t>(processed);
            progress.doubleValue = static_cast<double>(total);
            m_EventBus.Publish(progress);
        }
    }

    // Prune songs whose files disappeared from the scanned roots.
    int removed = 0;
    if (!m_Cancel.load())
    {
        Statement del = m_Db.Prepare("DELETE FROM songs WHERE id = ?;");
        for (const int64_t id : staleIds)
        {
            del.Bind(1, id);
            if (del.Step() == SQLITE_DONE)
            {
                ++removed;
            }
            del.Reset();
        }
        if (removed > 0)
        {
            flachead::core::Logger::Info("Library scan removed " + std::to_string(removed) +
                                         " stale song(s)");
            m_EventBus.Publish(Type::LibraryUpdated);
        }
    }

    Event finished;
    finished.type = Type::LibraryScanFinished;
    finished.intValue = static_cast<int64_t>(processed);
    m_EventBus.Publish(finished);

    m_Scanning = false;
    m_Cancel = false;
    flachead::core::Logger::Info("Library scan finished: " + std::to_string(processed) + "/" +
                                 std::to_string(total) + " files");
}

void LibraryScanner::ScanFile(const std::string& path, std::vector<int64_t>& staleIds)
{
    FileStat stat;
    if (!StatFile(path, stat))
    {
        return;
    }

    // Skip unchanged files; remember the id so it is not pruned.
    int64_t existingId = 0;
    {
        Statement lookup = m_Db.Prepare("SELECT id, file_mtime FROM songs WHERE path = ?;");
        lookup.Bind(1, path);
        if (lookup.Step() == SQLITE_ROW)
        {
            existingId = lookup.ColumnInt64(0);
            const int64_t existingMtime = lookup.ColumnInt64(1);
            if (existingMtime == stat.mtime)
            {
                MarkAlive(staleIds, existingId);
                return;
            }
        }
    }
    if (existingId > 0)
    {
        MarkAlive(staleIds, existingId);
    }

    TrackMetadata meta;
    const bool extracted = FFprobeExtractor{}.Extract(path, meta);

    SongModel song;
    song.path = path;
    song.folder = flachead::core::path::ParentDirectory(path);
    if (extracted)
    {
        song.title = meta.title;
        song.artist = meta.artist;
        song.album = meta.album;
        song.albumArtist = meta.albumArtist;
        song.genre = meta.genre;
        song.codec = meta.codec;
        song.trackNo = meta.trackNo;
        song.discNo = meta.discNo;
        song.year = meta.year;
        song.channels = meta.channels;
        song.sampleRate = meta.sampleRate;
        song.bitrate = meta.bitrate;
        song.duration = meta.duration;
        song.fileSize = meta.fileSize;
        song.fileMtime = meta.fileMtime;
        song.hasEmbeddedArt = meta.hasEmbeddedArt;
    }
    else
    {
        song.title = flachead::core::path::Stem(path);
        song.fileSize = stat.size;
        song.fileMtime = stat.mtime;
    }

    song.artPath = AlbumArtExtractor::EnsureCachedArt(
        song.path, song.folder, song.hasEmbeddedArt, AlbumArtExtractor::DefaultCacheDir());

    if (existingId > 0)
    {
        Statement upd = m_Db.Prepare(
            "UPDATE songs SET title=?, artist=?, album=?, album_artist=?, genre=?, folder=?, "
            "codec=?, track_no=?, disc_no=?, year=?, channels=?, sample_rate=?, bitrate=?, "
            "duration=?, file_size=?, file_mtime=?, has_art=?, art_path=? WHERE id=?;");
        upd.Bind(1, song.title);
        upd.Bind(2, song.artist);
        upd.Bind(3, song.album);
        upd.Bind(4, song.albumArtist);
        upd.Bind(5, song.genre);
        upd.Bind(6, song.folder);
        upd.Bind(7, song.codec);
        upd.Bind(8, song.trackNo);
        upd.Bind(9, song.discNo);
        upd.Bind(10, song.year);
        upd.Bind(11, song.channels);
        upd.Bind(12, song.sampleRate);
        upd.Bind(13, song.bitrate);
        upd.Bind(14, song.duration);
        upd.Bind(15, song.fileSize);
        upd.Bind(16, song.fileMtime);
        upd.Bind(17, song.hasEmbeddedArt ? 1 : 0);
        upd.Bind(18, song.artPath);
        upd.Bind(19, existingId);
        upd.Step();
        return;
    }

    Statement ins = m_Db.Prepare(
        "INSERT INTO songs (path, title, artist, album, album_artist, genre, folder, codec, "
        "track_no, disc_no, year, channels, sample_rate, bitrate, duration, file_size, "
        "file_mtime, date_added, has_art, art_path) "
        "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
    ins.Bind(1, song.path);
    ins.Bind(2, song.title);
    ins.Bind(3, song.artist);
    ins.Bind(4, song.album);
    ins.Bind(5, song.albumArtist);
    ins.Bind(6, song.genre);
    ins.Bind(7, song.folder);
    ins.Bind(8, song.codec);
    ins.Bind(9, song.trackNo);
    ins.Bind(10, song.discNo);
    ins.Bind(11, song.year);
    ins.Bind(12, song.channels);
    ins.Bind(13, song.sampleRate);
    ins.Bind(14, song.bitrate);
    ins.Bind(15, song.duration);
    ins.Bind(16, song.fileSize);
    ins.Bind(17, song.fileMtime);
    ins.Bind(18, NowSeconds());
    ins.Bind(19, song.hasEmbeddedArt ? 1 : 0);
    ins.Bind(20, song.artPath);
    ins.Step();
}
} // namespace flachead::library
