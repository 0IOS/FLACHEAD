#include "../src/library/LibraryService.hpp"
#include "../src/library/AlbumArtExtractor.hpp"
#include "../src/database/Database.hpp"
#include "../src/events/EventBus.hpp"
#include "../src/metadata/FFprobeExtractor.hpp"
#include "test_util.hpp"

#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using flachead::database::Database;
using flachead::events::EventBus;
using flachead::library::AlbumArtExtractor;
using flachead::library::LibraryService;
using flachead::metadata::FFprobeExtractor;
using flachead::models::SongModel;

namespace
{
constexpr const char* kRoot = "/tmp/flachead_lib_test";

bool Run(const std::string& command)
{
    return std::system(command.c_str()) == 0;
}

std::string MakeFlac(const std::string& path, const std::string& title, const std::string& artist,
                     const std::string& album, const std::string& track)
{
    Run("ffmpeg -y -loglevel error -f lavfi -i sine=frequency=440:duration=1 -c:a flac "
        "-metadata title=\"" + title + "\" -metadata artist=\"" + artist +
        "\" -metadata album=\"" + album + "\" -metadata track=\"" + track + "\" \"" + path +
        "\"");
    return path;
}

void TouchFile(const std::string& path)
{
    Run("touch \"" + path + "\"");
}
} // namespace

int main()
{
    Run("rm -rf " + std::string{kRoot});
    Run("mkdir -p " + std::string{kRoot});

    if (!FFprobeExtractor::Available())
    {
        Check(false, "ffprobe available");
        return Finish();
    }

    const std::string folder1 = std::string{kRoot} + "/AlbumX";
    const std::string folder2 = std::string{kRoot} + "/AlbumY";
    Run("mkdir -p \"" + folder1 + "\" \"" + folder2 + "\"");
    Run("ffmpeg -y -loglevel error -f lavfi -i color=c=red:s=64x64 -frames:v 1 \"" +
        folder1 + "/cover.png\"");

    const std::string fA = MakeFlac(folder1 + "/01_a.flac", "Song A1", "Artist One", "Album X", "1/2");
    const std::string fB = MakeFlac(folder1 + "/02_b.flac", "Song A2", "Artist One", "Album X", "2/2");
    const std::string fC = MakeFlac(folder2 + "/c.flac", "Song C1", "Artist Two", "Album Y", "1/1");

    Run("rm -f /tmp/flachead_lib_test.db");
    Database db;
    Check(db.Open("/tmp/flachead_lib_test.db"), "open library db");

    EventBus bus;
    int updatedCount = 0;
    bus.Subscribe(flachead::events::Type::LibraryUpdated,
                  [&](const flachead::events::Event&) { ++updatedCount; });

    LibraryService service(db, bus);
    Check(service.StartScan({kRoot}), "start scan");
    service.WaitForScan();
    Check(!service.IsScanning(), "scan finished");

    RunTest("scan populates library", [&] {
        Check(service.SongCount() == 3, "three songs found: " + std::to_string(service.SongCount()));

        const SongModel a = service.FindByPath(fA);
        Check(a.Valid(), "first song present");
        Check(a.title == "Song A1", "title: " + a.title);
        Check(a.artist == "Artist One", "artist: " + a.artist);
        Check(a.album == "Album X", "album: " + a.album);
        Check(a.trackNo == 1, "track number");

        Check(!service.FindByPath("/tmp/does_not_exist.flac").Valid(), "missing path not found");
    });

    RunTest("grouping queries", [&] {
        const auto albums = service.Albums();
        Check(albums.size() == 2, "two albums");
        bool foundX = false;
        for (const auto& album : albums)
        {
            if (album.name == "Album X")
            {
                foundX = true;
                Check(album.trackCount == 2, "album X has two tracks");
            }
        }
        Check(foundX, "album X present");

        const auto artists = service.Artists();
        Check(artists.size() == 2, "two artists");

        const auto albumTracks = service.AlbumTracks("Album X", "Artist One");
        Check(albumTracks.size() == 2, "album tracks");
        Check(albumTracks[0].title == "Song A1", "album ordered by track");
    });

    RunTest("search", [&] {
        Check(service.Search("Song A1").size() == 1, "search by title");
        Check(service.Search("Artist One").size() == 2, "search by artist");
        Check(service.Search("Album Y").size() == 1, "search by album");
        Check(service.Search("zzz").empty(), "no results for nonsense");
    });

    RunTest("favorites", [&] {
        const SongModel a = service.FindByPath(fA);
        Check(service.ToggleFavorite(a.id), "favorite a");
        const auto favorites = service.Favorites();
        Check(favorites.size() == 1, "one favorite");
        Check(favorites[0].id == a.id, "favorite is a");
        Check(service.ToggleFavorite(a.id), "unfavorite a");
        Check(service.Favorites().empty(), "favorites cleared");
    });

    RunTest("play counting + recent", [&] {
        const SongModel a = service.FindByPath(fA);
        service.MarkPlayed(a);
        service.MarkPlayed(a);
        const SongModel a2 = service.FindByPath(fA);
        Check(a2.playCount == 2, "play count incremented");
        const auto recent = service.Recent(5);
        Check(!recent.empty(), "recent list non-empty");
        Check(recent[0].id == a.id, "most recent first");
    });

    RunTest("album art cached", [&] {
        const SongModel a = service.FindByPath(fA);
        Check(!a.artPath.empty(), "art path assigned");
        if (!a.artPath.empty())
        {
            struct stat st {};
            Check(::stat(a.artPath.c_str(), &st) == 0, "cached art file exists");
        }
    });

    RunTest("incremental rescan skips unchanged", [&] {
        Check(service.StartScan({kRoot}), "start second scan");
        service.WaitForScan();
        Check(service.SongCount() == 3, "still three songs");
    });

    RunTest("changed file is re-extracted", [&] {
        TouchFile(fC);
        Check(service.StartScan({kRoot}), "start third scan");
        service.WaitForScan();
        const SongModel c = service.FindByPath(fC);
        Check(c.title == "Song C1", "metadata preserved after touch");
    });

    RunTest("deleted file is pruned", [&] {
        Run("rm -f \"" + fC + "\"");
        Check(service.StartScan({kRoot}), "start fourth scan");
        service.WaitForScan();
        Check(service.SongCount() == 2, "stale song pruned");
        Check(!service.FindByPath(fC).Valid(), "deleted song gone");
    });

    RunTest("art cache dir helper", [&] {
        Check(!AlbumArtExtractor::DefaultCacheDir().empty(), "cache dir derived");
        Check(AlbumArtExtractor::FindCoverInFolder(folder1) == folder1 + "/cover.png",
              "cover found in folder");
        Check(AlbumArtExtractor::FindCoverInFolder(folder2).empty(), "no cover in folder two");
    });

    db.Close();
    return Finish();
}
