#pragma once

#include "../database/Database.hpp"
#include "../events/EventBus.hpp"
#include "../models/AlbumModel.hpp"
#include "../models/ArtistModel.hpp"
#include "../models/SongModel.hpp"
#include "LibraryScanner.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace flachead::library
{
// Read/write facade over the songs database: search, album/artist grouping,
// recent and favorite queries, and play counting. Owns the background
// LibraryScanner. UI and playback go through this class; they never touch the
// Database directly.
class LibraryService
{
public:
    LibraryService(flachead::database::Database& db, flachead::events::EventBus& eventBus);
    ~LibraryService() = default;

    LibraryService(const LibraryService&) = delete;
    LibraryService& operator=(const LibraryService&) = delete;

    // --- Scanning ----------------------------------------------------------
    // Stores the scan roots and kicks off a background scan. Returns false
    // when a scan is already running.
    bool StartScan(const std::vector<std::string>& roots);
    bool IsScanning() const { return m_Scanner.IsScanning(); }
    void CancelScan() { m_Scanner.Cancel(); }
    void WaitForScan() { m_Scanner.Wait(); }
    int  LastScanCount() const { return m_Scanner.LastScanCount(); }

    // --- Queries -----------------------------------------------------------
    int  SongCount() const;
    std::vector<flachead::models::SongModel> AllSongs() const;
    std::vector<flachead::models::SongModel> SongsInFolder(const std::string& folder) const;
    std::vector<flachead::models::SongModel> Search(const std::string& query) const;
    std::vector<flachead::models::SongModel> Recent(int limit) const;
    std::vector<flachead::models::SongModel> RecentlyAdded(int limit) const;
    std::vector<flachead::models::SongModel> Favorites() const;
    std::vector<flachead::models::SongModel> DuplicateTitles() const;

    std::vector<flachead::models::AlbumModel>  Albums() const;
    std::vector<flachead::models::ArtistModel> Artists() const;
    std::vector<flachead::models::SongModel>   AlbumTracks(const std::string& album,
                                                           const std::string& artist) const;

    flachead::models::SongModel FindByPath(const std::string& path) const;
    flachead::models::SongModel FindById(std::int64_t id) const;

    // --- Mutations ---------------------------------------------------------
    bool ToggleFavorite(std::int64_t id);
    void MarkPlayed(const flachead::models::SongModel& song);

private:
    flachead::models::SongModel RowToSong(flachead::database::Statement& stmt) const;

    flachead::database::Database& m_Db;
    flachead::events::EventBus& m_EventBus;
    LibraryScanner m_Scanner;
};
} // namespace flachead::library
