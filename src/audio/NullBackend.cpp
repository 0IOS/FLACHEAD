#include "NullBackend.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <utility>

namespace flachead::audio
{
namespace
{
// Synthetic duration for virtual tracks; matches a "reasonable" album song.
constexpr double kFallbackDuration = 240.0;
constexpr double kPulseIntervalMs = 250.0;
} // namespace

void NullBackend::Shutdown()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Playing = false;
    m_Paused = true;
    m_MediaLoaded = false;
    m_Events.clear();
}

bool NullBackend::OpenMedia(std::string_view uri)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    Tick();

    m_Uri = std::string{uri};
    m_Position = 0.0;
    m_Duration = kFallbackDuration;
    m_Playing = false;
    m_Paused = true;
    m_MediaLoaded = true;
    m_Ended = false;

    m_Events.push_back(BackendEvent{BackendEvent::Kind::StartOfFile, m_Uri, 0, {}});
    m_Events.push_back(BackendEvent{BackendEvent::Kind::FileLoaded, m_Uri, 0, {}});
    return true;
}

bool NullBackend::QueueNext(std::string_view uri)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Events.push_back(BackendEvent{BackendEvent::Kind::None, std::string{uri}, 0, {}});
    return true;
}

void NullBackend::ClearQueue()
{
    // No queued playlist to manage in the null backend.
}

void NullBackend::Play()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    Tick();
    if (!m_MediaLoaded)
    {
        return;
    }
    if (m_Ended)
    {
        m_Position = 0.0;
        m_Ended = false;
    }
    m_Paused = false;
    m_Playing = true;
}

void NullBackend::Pause()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    Tick();
    if (m_Playing)
    {
        m_Paused = true;
        m_Playing = false;
    }
}

void NullBackend::Stop()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    Tick();
    m_Playing = false;
    m_Paused = true;
    m_MediaLoaded = false;
    m_Position = 0.0;
    m_Ended = false;
}

void NullBackend::Seek(double seconds)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    Tick();
    if (!m_MediaLoaded)
    {
        return;
    }
    m_Position = std::clamp(seconds, 0.0, m_Duration > 0.0 ? m_Duration : seconds);
}

void NullBackend::SetVolume(float linearVolume)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Volume = std::clamp(linearVolume, 0.0f, 1.0f);
}

void NullBackend::Tick() const
{
    if (!m_Playing)
    {
        m_LastTick = Clock::now();
        return;
    }

    const auto now = Clock::now();
    const double elapsed = std::chrono::duration<double>(now - m_LastTick).count();
    m_LastTick = now;

    m_Position += elapsed;

    // Emit an EndOfFile once the virtual track completes.
    if (m_MediaLoaded && !m_Ended && m_Duration > 0.0 && m_Position >= m_Duration)
    {
        m_Ended = true;
        m_Playing = false;
        m_Paused = true;
        m_Events.push_back(BackendEvent{BackendEvent::Kind::EndOfFile, m_Uri, 0, {}});
    }
}

double NullBackend::GetPositionSeconds() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    Tick();
    return m_Position;
}

double NullBackend::GetDurationSeconds() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Duration;
}

bool NullBackend::IsPlaying() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    Tick();
    return m_Playing;
}

bool NullBackend::IsPaused() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_Paused;
}

bool NullBackend::PollEvents(BackendEvent& out)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    Tick();
    if (m_Events.empty())
    {
        return false;
    }
    out = std::move(m_Events.front());
    m_Events.pop_front();
    return true;
}
} // namespace flachead::audio
