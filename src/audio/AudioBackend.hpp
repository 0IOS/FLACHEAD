#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace flachead::audio
{
// Events produced by a backend on its own thread. The service drains them
// from the main thread via PollEvents().
struct BackendEvent
{
    enum class Kind
    {
        None = 0,
        // A new media file has started loading (first frame of playback).
        StartOfFile,
        // Media finished loading and is ready to be played.
        FileLoaded,
        // Playback ended at EOF. The `reason` field carries the code.
        EndOfFile,
        // Playback hit a fatal error.
        Error,
        // Playback became idle (no media loaded).
        Idle,
    };

    Kind kind{Kind::None};
    std::string uri;
    int reason{0};
    std::string message;
};

// Abstraction over the playback engine. The UI never talks to a backend
// directly; it goes through AudioService.
//
// Backend contract:
// - OpenMedia() replaces the current media; the engine starts paused so the
//   caller decides when to start. Play() begins playback.
// - All methods must be callable from the main thread and must never block
//   longer than a few milliseconds; decoding/IO happens on the engine's own
//   threads.
// - Volume is linear 0.0..1.0, mapped to the backend's own scale.
// - Events are queued internally and drained from the main thread.
class AudioBackend
{
public:
    virtual ~AudioBackend() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    // Loads `uri` as the current media (replacing anything queued). The
    // engine starts paused; call Play() to begin.
    virtual bool OpenMedia(std::string_view uri) = 0;

    // Appends `uri` to the engine's internal queue for seamless (gapless)
    // transition after the current track finishes. Optional: returns false
    // when the backend cannot queue.
    virtual bool QueueNext(std::string_view uri) { return false; }

    // Removes anything queued after the current track.
    virtual void ClearQueue() {}

    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void Seek(double seconds) = 0;
    virtual void SetVolume(float linearVolume) = 0;

    virtual double GetPositionSeconds() const = 0;
    virtual double GetDurationSeconds() const = 0;
    virtual bool   IsPlaying() const = 0;
    virtual bool   IsPaused() const = 0;

    // Drains one queued event; returns false when the queue is empty.
    virtual bool PollEvents(BackendEvent& out) = 0;

    virtual std::string_view Name() const = 0;
};
} // namespace flachead::audio
