#include "../src/playback/PlaybackController.hpp"
#include "../src/playback/PlaylistEngine.hpp"
#include "../src/audio/AudioService.hpp"
#include "../src/audio/NullBackend.hpp"
#include "../src/events/EventBus.hpp"
#include "../src/database/Database.hpp"
#include "test_util.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>

using flachead::audio::AudioService;
using flachead::audio::NullBackend;
using flachead::database::Database;
using flachead::database::Statement;
using flachead::events::Event;
using flachead::events::EventBus;
using flachead::events::Type;
using flachead::models::SongModel;
using flachead::playback::PlaybackController;
using flachead::playback::PlaybackState;
using flachead::playback::PlaylistEngine;
using flachead::playback::QueueManager;
using flachead::playback::RepeatMode;

static void SleepMs(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

static SongModel MakeTrack(const std::string& path, const std::string& title)
{
    SongModel track;
    track.path = path;
    track.title = title;
    track.artist = "Test Artist";
    track.duration = 240.0;
    return track;
}

static std::vector<SongModel> MakeTracks(int count)
{
    std::vector<SongModel> tracks;
    for (int i = 0; i < count; ++i)
    {
        tracks.push_back(MakeTrack("/media/track_" + std::to_string(i) + ".flac",
                                   "Track " + std::to_string(i)));
    }
    return tracks;
}

// Drive playback to completion by ticking time and draining backend events.
static void Pump(EventBus& bus, AudioService& audio, int ms)
{
    SleepMs(ms);
    audio.PollBackendEvents();
}

// Seeds a real song row (playlist_tracks has a foreign key to songs) and
// returns its id.
static std::int64_t SeedSong(Database& db, const std::string& path, const std::string& title)
{
    db.Execute("DELETE FROM songs WHERE path = '" + path + "';");
    Statement insert = db.Prepare("INSERT INTO songs (path, title) VALUES (?, ?);");
    insert.Bind(1, path);
    insert.Bind(2, title);
    insert.Step();
    return db.LastInsertRowId();
}

int main()
{
    std::remove("/tmp/flachead_playlist_test.db");
    std::remove("/tmp/flachead_playlist_move_test.db");

    // --- QueueManager -------------------------------------------------------
    RunTest("queue navigation (repeat off)", [&] {
        QueueManager queue;
        queue.SetTracks(MakeTracks(3), 0);
        Check(queue.Size() == 3, "queue size");
        Check(queue.CurrentIndex() == 0, "start at requested index");
        Check(queue.HasNext(), "has next");
        Check(queue.Next() == 1, "advance to 1");
        Check(queue.Next() == 2, "advance to 2");
        Check(!queue.HasNext(), "no next at end");
        Check(queue.Next() == -1, "next at end returns -1");
        Check(queue.HasPrevious(), "has previous");
        Check(queue.Previous() == 1, "back to 1");
    });

    RunTest("queue repeat all", [&] {
        QueueManager queue;
        queue.SetTracks(MakeTracks(2), 0);
        queue.SetRepeat(RepeatMode::All);
        Check(queue.Next() == 1, "advance to 1");
        Check(queue.HasNext(), "wrap has next");
        Check(queue.Next() == 0, "wraps to 0");
    });

    RunTest("queue repeat one", [&] {
        QueueManager queue;
        queue.SetTracks(MakeTracks(2), 1);
        queue.SetRepeat(RepeatMode::One);
        Check(queue.Next() == 1, "repeat one stays");
        Check(queue.Next() == 1, "repeat one stays again");
    });

    RunTest("queue shuffle visits all", [&] {
        QueueManager queue;
        queue.SetTracks(MakeTracks(5), 0);
        queue.SetShuffle(true);
        std::vector<int> seen;
        for (int i = 0; i < 5; ++i)
        {
            seen.push_back(queue.CurrentIndex());
            if (i < 4)
            {
                queue.Next();
            }
        }
        std::sort(seen.begin(), seen.end());
        Check(seen == std::vector<int>({0, 1, 2, 3, 4}), "shuffle visits every track once");
    });

    RunTest("queue remove + insert + move", [&] {
        QueueManager queue;
        queue.SetTracks(MakeTracks(4), 0);
        queue.RemoveAt(1);
        Check(queue.Size() == 3, "remove shrinks queue");
        queue.InsertAfterCurrent(MakeTrack("/media/inserted.flac", "Inserted"));
        Check(queue.Size() == 4, "insert grows queue");
        Check(queue.Tracks()[1].title == "Inserted", "insert lands after current");
        queue.Move(3, 1);
        Check(queue.Tracks()[1].title == "Track 3", "move works");
    });

    // --- PlaylistEngine -----------------------------------------------------
    RunTest("playlist crud", [&] {
        EventBus bus;
        int changed = 0;
        bus.Subscribe(Type::PlaylistChanged, [&](const Event&) { ++changed; });

        Database db;
        Check(db.Open("/tmp/flachead_playlist_test.db"), "open playlist db");
        PlaylistEngine engine(db, bus);

        const int64_t playlistId = engine.CreatePlaylist("Favorites");
        Check(playlistId > 0, "create playlist");
        Check(engine.CreatePlaylist("Favorites") == playlistId, "create is idempotent by name");

        auto lists = engine.ListPlaylists();
        Check(lists.size() == 1, "one playlist listed");
        Check(lists[0].name == "Favorites", "playlist name");

        const int64_t songId = SeedSong(db, "/media/a.flac", "A");
        const int64_t songId2 = SeedSong(db, "/media/b.flac", "B");
        Check(engine.AddTrack(playlistId, songId), "add track");
        Check(engine.AddTrack(playlistId, songId2), "add second track");
        Check(engine.TrackIds(playlistId) == std::vector<int64_t>({songId, songId2}),
              "track order");

        Check(engine.RemoveTrackAt(playlistId, 0), "remove first track");
        Check(engine.TrackIds(playlistId) == std::vector<int64_t>({songId2}),
              "renumbered after remove");

        Check(engine.RenamePlaylist(playlistId, "Loved"), "rename playlist");
        Check(engine.PlaylistName(playlistId) == "Loved", "renamed playlist name");

        Check(engine.ClearPlaylist(playlistId), "clear playlist");
        Check(engine.TrackIds(playlistId).empty(), "playlist empty after clear");

        Check(engine.DeletePlaylist(playlistId), "delete playlist");
        Check(engine.ListPlaylists().empty(), "no playlists left");
        Check(changed >= 7, "playlist changed events published");
        db.Close();
    });

    RunTest("playlist move", [&] {
        EventBus bus;
        Database db;
        Check(db.Open("/tmp/flachead_playlist_move_test.db"), "open move db");
        PlaylistEngine engine(db, bus);

        const int64_t id = engine.CreatePlaylist("MoveMe");
        const int64_t one = SeedSong(db, "/media/1.flac", "1");
        const int64_t two = SeedSong(db, "/media/2.flac", "2");
        const int64_t three = SeedSong(db, "/media/3.flac", "3");
        engine.AddTrack(id, one);
        engine.AddTrack(id, two);
        engine.AddTrack(id, three);
        Check(engine.MoveTrack(id, 0, 2), "move 0 -> 2");
        Check(engine.TrackIds(id) == std::vector<int64_t>({two, three, one}), "order after move");
        db.Close();
    });

    // --- PlaybackController with the null backend --------------------------
    RunTest("controller plays through queue and finishes", [&] {
        EventBus bus;
        AudioService audio(bus, std::make_unique<NullBackend>());
        Check(audio.Initialize(), "audio init");

        PlaybackController controller(audio, bus);
        controller.Initialize();

        controller.PlayTracks(MakeTracks(3), 0);
        Check(controller.State() == PlaybackState::Loading, "loading after start");
        Pump(bus, audio, 20);
        Check(controller.State() == PlaybackState::Playing, "playing after start-of-file");
        Check(controller.CurrentIndex() == 0, "current index aligned");

        // Jump to the very end so the null backend fires EOF quickly.
        const double duration = audio.GetDurationSeconds();
        controller.Seek(duration - 0.1);
        Pump(bus, audio, 600);
        Check(controller.CurrentIndex() == 1, "auto-advanced to second track");
        Check(controller.State() == PlaybackState::Playing, "still playing after transition");

        controller.Seek(duration - 0.1);
        Pump(bus, audio, 600);
        Check(controller.CurrentIndex() == 2, "auto-advanced to third track");

        controller.Seek(duration - 0.1);
        Pump(bus, audio, 600);
        Check(controller.State() == PlaybackState::Finished, "finished at end of queue");

        controller.Shutdown();
        audio.Shutdown();
    });

    RunTest("controller repeat all wraps", [&] {
        EventBus bus;
        AudioService audio(bus, std::make_unique<NullBackend>());
        Check(audio.Initialize(), "audio init");

        PlaybackController controller(audio, bus);
        controller.Initialize();
        controller.SetRepeat(RepeatMode::All);
        controller.PlayTracks(MakeTracks(2), 0);
        Pump(bus, audio, 20);

        const double duration = audio.GetDurationSeconds();
        controller.Seek(duration - 0.1);
        Pump(bus, audio, 600);
        Check(controller.CurrentIndex() == 1, "wrapped track plays");
        Check(controller.State() == PlaybackState::Playing, "repeat all keeps playing");

        controller.Shutdown();
        audio.Shutdown();
    });

    RunTest("controller repeat one replays", [&] {
        EventBus bus;
        AudioService audio(bus, std::make_unique<NullBackend>());
        Check(audio.Initialize(), "audio init");

        PlaybackController controller(audio, bus);
        controller.Initialize();
        controller.SetRepeat(RepeatMode::One);
        controller.PlayTracks(MakeTracks(2), 0);
        Pump(bus, audio, 20);

        const double duration = audio.GetDurationSeconds();
        controller.Seek(duration - 0.1);
        Pump(bus, audio, 600);
        Check(controller.CurrentIndex() == 0, "repeat one replays same track");
        Check(controller.State() == PlaybackState::Playing, "repeat one playing");

        controller.Shutdown();
        audio.Shutdown();
    });

    RunTest("controller next / previous / toggle / stop", [&] {
        EventBus bus;
        AudioService audio(bus, std::make_unique<NullBackend>());
        Check(audio.Initialize(), "audio init");

        PlaybackController controller(audio, bus);
        controller.Initialize();
        controller.PlayTracks(MakeTracks(3), 0);
        Pump(bus, audio, 20);

        controller.Pause();
        Check(controller.State() == PlaybackState::Paused, "paused");
        controller.Toggle();
        Check(controller.State() == PlaybackState::Playing, "resumed via toggle");

        controller.Next();
        Pump(bus, audio, 20);
        Check(controller.CurrentIndex() == 1, "next advances");
        Check(controller.State() == PlaybackState::Playing, "playing next");

        controller.Previous();
        Pump(bus, audio, 20);
        Check(controller.CurrentIndex() == 0, "previous goes back");

        controller.Stop();
        Check(controller.State() == PlaybackState::Stopped, "stopped");

        controller.Shutdown();
        audio.Shutdown();
    });

    RunTest("controller volume", [&] {
        EventBus bus;
        AudioService audio(bus, std::make_unique<NullBackend>());
        Check(audio.Initialize(), "audio init");

        float stored = -1.0f;
        PlaybackController controller(audio, bus);
        controller.SetVolumeStore([&](float v) { stored = v; });
        controller.SetVolume(0.35f);
        Check(std::abs(stored - 0.35f) < 0.001f, "volume persisted via store");
        Check(std::abs(audio.GetVolume() - 0.35f) < 0.001f, "volume applied to backend");

        audio.Shutdown();
    });

    return Finish();
}
