#include "PlaybackController.hpp"

#include "../core/Logger.hpp"
#include "../core/TimeFormat.hpp"

#include <algorithm>
#include <utility>

namespace flachead::playback
{
using flachead::audio::AudioService;
using flachead::events::Event;
using flachead::events::EventBus;
using flachead::events::Type;
using flachead::models::SongModel;

const char* StateName(PlaybackState state)
{
    switch (state)
    {
        case PlaybackState::Stopped: return "Stopped";
        case PlaybackState::Loading: return "Loading";
        case PlaybackState::Playing: return "Playing";
        case PlaybackState::Paused: return "Paused";
        case PlaybackState::Seeking: return "Seeking";
        case PlaybackState::Buffering: return "Buffering";
        case PlaybackState::Finished: return "Finished";
        case PlaybackState::Error: return "Error";
    }
    return "Unknown";
}

PlaybackController::PlaybackController(AudioService& audio, EventBus& eventBus)
    : m_Audio(audio),
      m_EventBus(eventBus)
{
}

void PlaybackController::Initialize()
{
    Subscribe();
}

void PlaybackController::Shutdown()
{
    if (m_EventSubscription > 0)
    {
        m_EventBus.Unsubscribe(m_EventSubscription);
        m_EventSubscription = 0;
    }
}

void PlaybackController::Subscribe()
{
    const Type types[] = {Type::TrackChanged, Type::PlaybackFinished, Type::PlaybackStopped,
                          Type::PlaybackError};
    for (const Type type : types)
    {
        m_EventBus.Subscribe(type, [this](const Event& event) { OnAudioEvent(event); });
    }
}

void PlaybackController::Update(float)
{
    if (m_State != PlaybackState::Playing && m_State != PlaybackState::Paused)
    {
        return;
    }

    // Throttled progress event for subscribers that want periodic updates.
    const double position = m_Audio.GetPositionSeconds();
    if (position - m_LastProgressEmit >= 1.0)
    {
        m_LastProgressEmit = position;
        Event event;
        event.type = Type::TrackProgress;
        event.doubleValue = position;
        m_EventBus.Publish(event);
    }
}

void PlaybackController::SetState(PlaybackState state)
{
    if (m_State == state)
    {
        return;
    }
    flachead::core::Logger::Debug("Playback state: " + std::string{StateName(m_State)} + " -> " +
                                  StateName(state));
    m_State = state;
}

void PlaybackController::RecordPlay()
{
    if (m_PlayHook && m_CurrentTrack.Valid())
    {
        m_PlayHook(m_CurrentTrack);
    }
}

void PlaybackController::StartTrackAt(int index)
{
    if (m_Queue.IsEmpty())
    {
        Stop();
        return;
    }

    m_Queue.PlayAt(index);
    m_CurrentTrack = m_Queue.CurrentTrack();
    if (!m_CurrentTrack.Valid())
    {
        Stop();
        return;
    }

    m_PreloadQueued = false;
    m_WaitingForAutoAdvance = false;
    SetState(PlaybackState::Loading);

    if (!m_Audio.PlayTrack(m_CurrentTrack.path))
    {
        HandleError("failed to start playback");
        return;
    }
}

void PlaybackController::PlayTrack(const SongModel& track)
{
    m_Queue.SetTracks({track}, 0);
    StartTrackAt(0);
}

void PlaybackController::PlayTracks(const std::vector<SongModel>& tracks, int startIndex)
{
    m_Queue.SetTracks(tracks, startIndex);
    StartTrackAt(startIndex);
}

void PlaybackController::PlayNext(const SongModel& track)
{
    m_Queue.InsertAfterCurrent(track);
    m_EventBus.Publish(Type::QueueChanged);
}

void PlaybackController::Toggle()
{
    switch (m_State)
    {
        case PlaybackState::Playing:
            m_Audio.Pause();
            SetState(PlaybackState::Paused);
            break;
        case PlaybackState::Paused:
            m_Audio.Play();
            SetState(PlaybackState::Playing);
            break;
        case PlaybackState::Stopped:
        case PlaybackState::Finished:
            if (m_CurrentTrack.Valid())
            {
                PlayTrack(m_CurrentTrack);
            }
            break;
        default:
            break;
    }
}

void PlaybackController::Pause()
{
    if (m_State == PlaybackState::Playing)
    {
        m_Audio.Pause();
        SetState(PlaybackState::Paused);
    }
}

void PlaybackController::Resume()
{
    if (m_State == PlaybackState::Paused)
    {
        m_Audio.Play();
        SetState(PlaybackState::Playing);
    }
}

void PlaybackController::Stop()
{
    m_PreloadQueued = false;
    m_WaitingForAutoAdvance = false;
    // Set the state first: AudioService::Stop() publishes PlaybackStopped
    // synchronously and HandleStopped() must not recurse.
    SetState(PlaybackState::Stopped);
    m_Audio.Stop();
}

void PlaybackController::Next()
{
    if (m_Queue.IsEmpty())
    {
        return;
    }

    const int next = m_Queue.Next();
    if (next >= 0)
    {
        StartTrackAt(next);
    }
    else
    {
        FinishPlayback();
    }
}

void PlaybackController::Previous()
{
    if (m_Queue.IsEmpty())
    {
        return;
    }

    // Restart the current track when more than a few seconds have played.
    if (m_Audio.GetPositionSeconds() > 3.0)
    {
        StartTrackAt(m_Queue.CurrentIndex());
        return;
    }

    const int previous = m_Queue.Previous();
    if (previous >= 0)
    {
        StartTrackAt(previous);
    }
    else
    {
        StartTrackAt(m_Queue.CurrentIndex());
    }
}

void PlaybackController::Seek(double seconds)
{
    if (m_State == PlaybackState::Stopped || m_State == PlaybackState::Finished)
    {
        return;
    }
    const PlaybackState previous = m_State;
    SetState(PlaybackState::Seeking);
    m_Audio.Seek(seconds);
    // mpv reports PLAYBACK_RESTART after the seek completes; the null backend
    // resumes immediately. Restore the pre-seek state optimistically.
    SetState(previous);
}

void PlaybackController::SetRepeat(RepeatMode mode)
{
    m_Queue.SetRepeat(mode);
    m_EventBus.Publish(Type::RepeatChanged, static_cast<int64_t>(mode));
}

void PlaybackController::ToggleRepeat()
{
    switch (m_Queue.Repeat())
    {
        case RepeatMode::Off: SetRepeat(RepeatMode::All); break;
        case RepeatMode::All: SetRepeat(RepeatMode::One); break;
        case RepeatMode::One: SetRepeat(RepeatMode::Off); break;
    }
}

void PlaybackController::ToggleShuffle()
{
    m_Queue.SetShuffle(!m_Queue.Shuffle());
    m_EventBus.Publish(Type::ShuffleChanged, m_Queue.Shuffle() ? 1 : 0);
}

void PlaybackController::RemoveFromQueue(int index)
{
    m_Queue.RemoveAt(index);
    m_EventBus.Publish(Type::QueueChanged);
}

void PlaybackController::ClearQueue()
{
    m_Queue.Clear();
    m_EventBus.Publish(Type::QueueChanged);
}

void PlaybackController::SetVolume(float linear)
{
    m_Volume = std::clamp(linear, 0.0f, 1.0f);
    m_Audio.SetVolume(m_Volume);
    if (m_VolumeStore)
    {
        m_VolumeStore(m_Volume);
    }
}

void PlaybackController::PreloadNext()
{
    if (m_Queue.Repeat() == RepeatMode::One || !m_Queue.HasNext())
    {
        m_Audio.ClearQueue();
        m_PreloadQueued = false;
        return;
    }

    const int nextIndex = m_Queue.PeekNext();
    if (nextIndex < 0)
    {
        m_Audio.ClearQueue();
        m_PreloadQueued = false;
        return;
    }

    const SongModel& next = m_Queue.Tracks()[static_cast<std::size_t>(nextIndex)];
    m_PreloadQueued = m_Audio.QueueNext(next.path);
    if (!m_PreloadQueued)
    {
        m_Audio.ClearQueue();
    }
}

void PlaybackController::HandleTrackChanged(const std::string& uri)
{
    // Align the queue index with the engine's current file (matters for
    // gapless auto-advance and playlist navigation).
    if (!m_Queue.IsEmpty())
    {
        for (int i = 0; i < m_Queue.Size(); ++i)
        {
            if (m_Queue.Tracks()[static_cast<std::size_t>(i)].path == uri)
            {
                m_Queue.PlayAt(i);
                break;
            }
        }
    }

    m_CurrentTrack = m_Queue.CurrentTrack();
    if (!m_CurrentTrack.Valid() || m_CurrentTrack.path != uri)
    {
        SongModel fallback;
        fallback.path = uri;
        m_CurrentTrack = std::move(fallback);
    }

    m_PreloadQueued = false;
    m_WaitingForAutoAdvance = false;
    SetState(PlaybackState::Playing);
    RecordPlay();
    PreloadNext();
}

void PlaybackController::HandleEndOfFile()
{
    if (m_State == PlaybackState::Stopped || m_State == PlaybackState::Finished ||
        m_State == PlaybackState::Error)
    {
        return;
    }

    if (m_Queue.Repeat() == RepeatMode::One && m_CurrentTrack.Valid())
    {
        // Replay the same track.
        StartTrackAt(m_Queue.CurrentIndex());
        return;
    }

    if (m_Queue.HasNext())
    {
        if (m_PreloadQueued)
        {
            // The engine auto-advances gaplessly; TrackChanged will arrive.
            m_WaitingForAutoAdvance = true;
            SetState(PlaybackState::Playing);
            return;
        }

        // No gapless preload was in place; advance explicitly.
        const int next = m_Queue.Next();
        if (next >= 0)
        {
            StartTrackAt(next);
        }
        else
        {
            FinishPlayback();
        }
        return;
    }

    FinishPlayback();
}

void PlaybackController::FinishPlayback()
{
    m_PreloadQueued = false;
    m_WaitingForAutoAdvance = false;
    // Set the state first so the synchronous PlaybackStopped from Stop() is
    // ignored by HandleStopped().
    SetState(PlaybackState::Finished);
    m_Audio.Stop();
    m_EventBus.Publish(Type::PlaybackFinished);
}

void PlaybackController::HandleError(const std::string& message)
{
    flachead::core::Logger::Error("Playback error: " + message);
    m_PreloadQueued = false;
    m_WaitingForAutoAdvance = false;
    SetState(PlaybackState::Error);
    Event event;
    event.type = Type::PlaybackError;
    event.stringValue = message;
    m_EventBus.Publish(event);
}

void PlaybackController::HandleStopped()
{
    // Stop()/FinishPlayback() publish PlaybackStopped while already settling;
    // ignore those. A spontaneous idle from the engine only matters when we
    // expected a gapless transition that never happened.
    if (m_State == PlaybackState::Stopped || m_State == PlaybackState::Finished ||
        m_State == PlaybackState::Error)
    {
        return;
    }

    if (m_WaitingForAutoAdvance && m_Queue.HasNext())
    {
        m_WaitingForAutoAdvance = false;
        const int next = m_Queue.Next();
        if (next >= 0)
        {
            StartTrackAt(next);
            return;
        }
    }
    FinishPlayback();
}

void PlaybackController::OnAudioEvent(const Event& event)
{
    switch (event.type)
    {
        case Type::TrackChanged:
            HandleTrackChanged(event.stringValue);
            break;
        case Type::PlaybackFinished:
            HandleEndOfFile();
            break;
        case Type::PlaybackStopped:
            HandleStopped();
            break;
        case Type::PlaybackError:
            HandleError(event.stringValue.empty() ? "playback error" : event.stringValue);
            break;
        default:
            break;
    }
}
} // namespace flachead::playback
