#include "../src/audio/AudioService.hpp"
#include "../src/events/EventBus.hpp"
#include "test_util.hpp"

#include <chrono>
#include <cmath>
#include <thread>

using flachead::audio::AudioService;
using flachead::audio::NullBackend;
using flachead::events::Event;
using flachead::events::EventBus;
using flachead::events::Type;

static void SleepMs(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

int main()
{
    EventBus bus;
    AudioService audio(bus, std::make_unique<NullBackend>());
    Check(audio.Initialize(), "audio service initialize");
    Check(audio.BackendName() == "null", "null backend selected on host without libmpv");

    int finishedCount = 0;
    bus.Subscribe(Type::PlaybackFinished, [&](const Event&) { ++finishedCount; });

    RunTest("play / pause / resume", [&] {
        Check(audio.PlayTrack("/tmp/test_song.flac"), "play track");
        Check(audio.IsPlaying(), "playing after play");
        Check(!audio.IsPaused(), "not paused while playing");

        SleepMs(80);
        audio.PollBackendEvents();
        const double pos1 = audio.GetPositionSeconds();
        Check(pos1 > 0.0, "position advances: " + std::to_string(pos1));

        audio.Pause();
        Check(!audio.IsPlaying(), "stopped playing after pause");
        Check(audio.IsPaused(), "paused flag set");

        const double posPaused = audio.GetPositionSeconds();
        SleepMs(60);
        Check(std::abs(audio.GetPositionSeconds() - posPaused) < 0.01, "position frozen while paused");

        audio.Play();
        Check(audio.IsPlaying(), "resumed");
        SleepMs(60);
        Check(audio.GetPositionSeconds() > posPaused, "position advances again");
    });

    RunTest("seek + duration", [&] {
        audio.Seek(30.0);
        SleepMs(30);
        const double pos = audio.GetPositionSeconds();
        Check(std::abs(pos - 30.0) < 0.5, "seek to 30s: " + std::to_string(pos));
        Check(audio.GetDurationSeconds() > 0.0, "duration known");
    });

    RunTest("volume", [&] {
        audio.SetVolume(0.5f);
        Check(std::abs(audio.GetVolume() - 0.5f) < 0.001f, "volume stored");
        audio.SetVolume(2.0f);
        Check(std::abs(audio.GetVolume() - 1.0f) < 0.001f, "volume clamped");
        audio.SetVolume(-1.0f);
        Check(std::abs(audio.GetVolume() - 0.0f) < 0.001f, "volume floor");
        audio.SetVolume(0.8f);
    });

    RunTest("queue next", [&] {
        Check(audio.QueueNext("/tmp/test_song.flac"), "queue next accepted");
        audio.ClearQueue();
    });

    RunTest("end of file event", [&] {
        const double duration = audio.GetDurationSeconds();
        audio.Seek(duration - 0.1);
        SleepMs(400);
        audio.PollBackendEvents();
        Check(finishedCount >= 1, "playback finished event fired");
    });

    RunTest("stop", [&] {
        audio.PlayTrack("/tmp/test_song.flac");
        audio.Stop();
        Check(!audio.IsPlaying(), "stopped");
    });

    audio.Shutdown();
    return Finish();
}
