#pragma once

#include "../audio/AudioService.hpp"
#include "../events/EventBus.hpp"
#include "../models/SongModel.hpp"
#include "QueueManager.hpp"

#include <functional>
#include <string>
#include <vector>

namespace flachead::playback
{
enum class PlaybackState
{
    Stopped = 0,
    Loading,
    Playing,
    Paused,
    Seeking,
    Buffering,
    Finished,
    Error,
};

const char* StateName(PlaybackState state);

// Owns the playback state machine and queue. UI commands land here; the
// controller talks to AudioService and the EventBus and never leaks backend
// internals.
//
// State machine:
//   Stopped -> Loading -> Playing <-> Paused
//   Playing -> Seeking -> Playing
//   Playing -> Finished (end of queue)
//   Playing/Loading -> Error
class PlaybackController
{
public:
    PlaybackController(flachead::audio::AudioService& audio,
                       flachead::events::EventBus& eventBus);

    void Initialize();
    void Shutdown();
    void Update(float deltaSeconds);

    // Playback control.
    void PlayTrack(const flachead::models::SongModel& track);
    void PlayTracks(const std::vector<flachead::models::SongModel>& tracks, int startIndex = 0);
    void Toggle();
    void Pause();
    void Resume();
    void Stop();
    void Next();
    void Previous();
    void Seek(double seconds);

    // Volume (persisted through the volume store hook when set).
    void SetVolume(float linear);
    float Volume() const { return m_Volume; }
    void SetVolumeStore(std::function<void(float)> store) { m_VolumeStore = std::move(store); }

    // Played-track hook (play counting / history), registered by the library.
    void SetPlayHook(std::function<void(const flachead::models::SongModel&)> hook)
    {
        m_PlayHook = std::move(hook);
    }

    // Queue control.
    void SetRepeat(RepeatMode mode);
    void ToggleRepeat();
    void ToggleShuffle();
    void PlayNext(const flachead::models::SongModel& track);
    void RemoveFromQueue(int index);
    void ClearQueue();

    // State queries.
    PlaybackState State() const { return m_State; }
    const flachead::models::SongModel& CurrentTrack() const { return m_CurrentTrack; }
    int CurrentIndex() const { return m_Queue.CurrentIndex(); }
    const QueueManager& Queue() const { return m_Queue; }
    bool IsPlaying() const { return m_State == PlaybackState::Playing; }
    double PositionSeconds() const { return m_Audio.GetPositionSeconds(); }
    double DurationSeconds() const { return m_Audio.GetDurationSeconds(); }
    std::string_view BackendName() const { return m_Audio.BackendName(); }

private:
    void Subscribe();
    void OnAudioEvent(const flachead::events::Event& event);
    void HandleTrackChanged(const std::string& uri);
    void HandleEndOfFile();
    void HandleError(const std::string& message);
    void HandleStopped();
    void PreloadNext();
    void StartTrackAt(int index);
    void SetState(PlaybackState state);
    void RecordPlay();
    void FinishPlayback();

    flachead::audio::AudioService& m_Audio;
    flachead::events::EventBus& m_EventBus;

    QueueManager m_Queue;
    flachead::models::SongModel m_CurrentTrack;

    PlaybackState m_State{PlaybackState::Stopped};
    int m_EventSubscription{0};

    float m_Volume{0.8f};
    std::function<void(float)> m_VolumeStore;
    std::function<void(const flachead::models::SongModel&)> m_PlayHook;

    bool m_PreloadQueued{false};
    bool m_WaitingForAutoAdvance{false};
    double m_LastProgressEmit{-1.0};
};
} // namespace flachead::playback
