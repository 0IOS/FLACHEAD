#pragma once

#include "../database/Database.hpp"
#include "../events/EventBus.hpp"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace flachead::library
{
// Recursively scans the configured music folders, extracts metadata with the
// ffprobe extractor, and upserts songs into the library database. Runs on a
// background thread so the UI stays responsive; progress is reported through
// LibraryScanStarted / LibraryScanProgress / LibraryScanFinished and a final
// LibraryUpdated when the library changed.
//
// The scan is incremental: files whose (mtime, size) match the database are
// skipped, so rescans are cheap. Songs under the scanned roots that disappear
// from disk are pruned.
class LibraryScanner
{
public:
    LibraryScanner(flachead::database::Database& db, flachead::events::EventBus& eventBus);
    ~LibraryScanner();

    LibraryScanner(const LibraryScanner&) = delete;
    LibraryScanner& operator=(const LibraryScanner&) = delete;

    // Starts a background scan of `roots`. Returns false when a scan is
    // already running or no roots are given.
    bool StartScan(const std::vector<std::string>& roots);
    void Cancel();
    void Wait();

    bool IsScanning() const { return m_Scanning.load(); }
    int  LastScanCount() const { return m_LastCount.load(); }

private:
    void ScanWorker(const std::vector<std::string>& roots);
    void ScanFile(const std::string& path, std::vector<int64_t>& staleIds);

    flachead::database::Database& m_Db;
    flachead::events::EventBus& m_EventBus;

    std::atomic<bool> m_Scanning{false};
    std::atomic<bool> m_Cancel{false};
    std::atomic<int> m_LastCount{0};
    std::thread m_Thread;
};
} // namespace flachead::library
