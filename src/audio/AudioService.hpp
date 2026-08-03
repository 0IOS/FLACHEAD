#pragma once

#include "AudioBackend.hpp"
#include "MpvBackend.hpp"
#include "NullBackend.hpp"
#include "../events/EventBus.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace flachead::audio
{
// The only audio entry point the UI and playback layer touch. Owns a backend
// (libmpv when available, otherwise the null simulator) and translates
// backend events into EventBus events:
//
//   StartOfFile  -> TrackChanged
//   FileLoaded   -> TrackLoaded
//   EndOfFile    -> PlaybackFinished / PlaybackStopped / PlaybackError
//   Play/Pause   -> PlaybackStarted / PlaybackPaused / PlaybackResumed
//
// Call PollBackendEvents() once per frame from the main loop.
class AudioService
{
public:
    explicit AudioService(flachead::events::EventBus& eventBus,
                          std::unique_ptr<AudioBackend> backend = nullptr);

    bool Initialize();
    void Shutdown();

    // Drains pending backend events and publishes them on the event bus.
    void PollBackendEvents();

    // Loads and starts playback of `uri`.
    bool PlayTrack(std::string_view uri);
    // Loads `uri` paused; call Play() to begin.
    bool LoadTrack(std::string_view uri);
    // Queues `uri` for gapless transition after the current track.
    bool QueueNext(std::string_view uri);
    void ClearQueue();

    void Play();
    void Pause();
    void Toggle();
    void Stop();
    void Seek(double seconds);

    void SetVolume(float linear);
    float GetVolume() const { return m_Volume; }

    double GetPositionSeconds() const;
    double GetDurationSeconds() const;
    bool   IsPlaying() const;
    bool   IsPaused() const;

    std::string_view BackendName() const;
    bool BackendAvailable() const { return m_BackendAvailable; }

private:
    std::unique_ptr<AudioBackend> MakeDefaultBackend() const;

    flachead::events::EventBus& m_EventBus;
    std::unique_ptr<AudioBackend> m_Backend;
    bool m_BackendAvailable{false};
    float m_Volume{1.0f};
    std::string m_CurrentUri;
};
} // namespace flachead::audio
