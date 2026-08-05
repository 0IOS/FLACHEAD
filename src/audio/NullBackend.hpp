#pragma once

#include "AudioBackend.hpp"

#include <chrono>
#include <deque>
#include <mutex>

namespace flachead::audio
{
// Simulated playback backend used on hosts without libmpv and for UI/state
// testing. Plays a virtual clock: position advances while playing, duration
// is synthesized, and EndOfFile fires when the clock reaches the end.
class NullBackend final : public AudioBackend
{
public:
    bool Initialize() override { return true; }
    void Shutdown() override;

    bool OpenMedia(std::string_view uri) override;
    bool QueueNext(std::string_view uri) override;
    void ClearQueue() override;
    void Play() override;
    void Pause() override;
    void Stop() override;
    void Seek(double seconds) override;
    void SetVolume(float linearVolume) override;

    double GetPositionSeconds() const override;
    double GetDurationSeconds() const override;
    bool   IsPlaying() const override;
    bool   IsPaused() const override;

    bool PollEvents(BackendEvent& out) override;

    std::string_view Name() const override { return "null"; }

private:
    using Clock = std::chrono::steady_clock;

    // Advances the virtual clock; called from all queries and polls.
    void Tick() const;

    mutable std::mutex m_Mutex;
    mutable std::deque<BackendEvent> m_Events;
    mutable Clock::time_point m_LastTick{Clock::now()};
    mutable std::string m_Uri;
    mutable double m_Position{0.0};
    double m_Duration{0.0};
    mutable bool m_Playing{false};
    mutable bool m_Paused{true};
    bool m_MediaLoaded{false};
    mutable bool m_Ended{false};
    mutable std::string m_QueuedUri;
    float m_Volume{1.0f};
};
} // namespace flachead::audio
