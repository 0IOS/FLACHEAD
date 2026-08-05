#include "../src/audio/AudioService.hpp"
#include "../src/audio/NullBackend.hpp"
#include "../src/database/Database.hpp"
#include "../src/events/EventBus.hpp"
#include "../src/library/LibraryService.hpp"
#include "../src/models/SongModel.hpp"
#include "../src/playback/PlaybackController.hpp"
#include "../src/playback/QueueManager.hpp"
#include "../src/services/SettingsManager.hpp"
#include "test_util.hpp"

#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

using flachead::audio::AudioService;
using flachead::audio::NullBackend;
using flachead::database::Database;
using flachead::database::Statement;
using flachead::events::EventBus;
using flachead::library::LibraryService;
using flachead::models::SongModel;
using flachead::playback::PlaybackController;
using flachead::playback::PlaybackState;
using flachead::services::SettingsManager;

static void SleepMs(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Mirrors Application::SetupServices wiring: settings-backed volume, library
// play counting, and the main-loop pump (PollBackendEvents + Update).
int main()
{
    const std::string dbPath = "/tmp/flachead_app_wiring.db";
    std::remove(dbPath.c_str());

    Database db;
    Check(db.Open(dbPath), "open wiring db");
    Check(db.Migrate(), "migrate wiring db");

    for (int i = 0; i < 3; ++i)
    {
        Statement insert = db.Prepare(
            "INSERT INTO songs (path, title, artist, album, duration) "
            "VALUES (?, ?, ?, ?, 240.0);");
        insert.Bind(1, "/media/wiring_" + std::to_string(i) + ".flac");
        insert.Bind(2, "Wiring " + std::to_string(i));
        insert.Bind(3, "Integration");
        insert.Bind(4, "Wiring Test");
        insert.Step();
    }

    EventBus bus;
    SettingsManager settings;
    settings.Initialize(db, bus);

    AudioService audio(bus, std::make_unique<NullBackend>());
    Check(audio.Initialize(), "null backend init");

    LibraryService library(db, bus);
    PlaybackController playback(audio, bus);
    playback.Initialize();

    // The exact hooks Application installs.
    playback.SetVolumeStore([&](float v) { settings.SetFloat("audio.volume", v); });
    playback.SetPlayHook([&](const SongModel& song) { library.MarkPlayed(song); });

    // Volume is persisted through settings.
    playback.SetVolume(0.42f);
    Check(settings.GetFloat("audio.volume") == 0.42f, "volume persisted to settings");

    // Play the whole seeded library through the real services.
    const auto songs = library.AllSongs();
    Check(songs.size() == 3, "library sees 3 seeded songs");
    playback.PlayTracks(songs, 0);

    // Pump until loading completes.
    bool reachedPlaying = false;
    for (int i = 0; i < 20 && !reachedPlaying; ++i)
    {
        SleepMs(50);
        audio.PollBackendEvents();
        playback.Update(0.05f);
        reachedPlaying = playback.State() == PlaybackState::Playing;
    }
    Check(reachedPlaying, "first track reaches Playing");

    // Advancing through the queue triggers the play hook (play counting).
    auto playedCount = [&](int index) {
        const auto song = library.FindByPath("/media/wiring_" + std::to_string(index) + ".flac");
        return song.playCount;
    };
    Check(playedCount(0) >= 1, "track 0 counted as played");

    playback.Next();
    for (int i = 0; i < 20 && playback.State() != PlaybackState::Playing; ++i)
    {
        SleepMs(50);
        audio.PollBackendEvents();
        playback.Update(0.05f);
    }
    Check(playback.CurrentIndex() == 1, "Next() moves to track 1");
    Check(playback.State() == PlaybackState::Playing, "track 1 playing");
    Check(playedCount(1) >= 1, "track 1 counted as played");

    // Favorite toggling reflects in library queries.
    const auto favTarget = library.FindByPath("/media/wiring_0.flac");
    Check(favTarget.Valid(), "find favorite target");
    library.ToggleFavorite(favTarget.id);
    Check(library.Favorites().size() == 1, "favorite recorded");
    library.ToggleFavorite(favTarget.id);
    Check(library.Favorites().empty(), "favorite toggled off");

    // Settings survive a fresh manager over the same database.
    SettingsManager reloaded;
    reloaded.Initialize(db, bus);
    Check(reloaded.GetFloat("audio.volume") == 0.42f, "volume survives reload");

    // End of queue publishes PlaybackFinished (clean finish, no crash).
    playback.PlayTracks(songs, 0);
    for (int t = 0; t < 3; ++t)
    {
        for (int i = 0; i < 20 && playback.State() != PlaybackState::Playing; ++i)
        {
            SleepMs(25);
            audio.PollBackendEvents();
            playback.Update(0.025f);
        }
        Check(playback.State() == PlaybackState::Playing, "track resumed after advance");
        playback.Seek(239.9);
        for (int i = 0; i < 20; ++i)
        {
            SleepMs(25);
            audio.PollBackendEvents();
            playback.Update(0.025f);
            if (playback.CurrentIndex() != t)
            {
                break;
            }
        }
    }
    Check(playback.State() == PlaybackState::Finished, "queue finishes cleanly");

    playback.Shutdown();
    audio.Shutdown();
    library.WaitForScan();
    db.Close();
    return Finish();
}
