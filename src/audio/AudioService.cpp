#include "AudioService.hpp"

#include "../core/Logger.hpp"

#include <algorithm>
#include <utility>

namespace flachead::audio
{
namespace
{
// mpv end-file reasons; see <mpv/client.h>.
constexpr int kEndFileEof = 0;
constexpr int kEndFileStop = 2;
constexpr int kEndFileError = 4;
} // namespace

AudioService::AudioService(flachead::events::EventBus& eventBus,
                           std::unique_ptr<AudioBackend> backend)
    : m_EventBus(eventBus),
      m_Backend(std::move(backend))
{
}

std::unique_ptr<AudioBackend> AudioService::MakeDefaultBackend() const
{
    if (MpvBackend::Available())
    {
        return std::make_unique<MpvBackend>();
    }
    return std::make_unique<NullBackend>();
}

bool AudioService::Initialize()
{
    if (!m_Backend)
    {
        m_Backend = MakeDefaultBackend();
    }

    m_BackendAvailable = m_Backend->Initialize();
    if (!m_BackendAvailable)
    {
        flachead::core::Logger::Warning("Audio backend '" + std::string{m_Backend->Name()} +
                                        "' failed to initialize");
        return false;
    }

    m_Backend->SetVolume(m_Volume);
    flachead::core::Logger::Info("Audio backend initialized: " + std::string{m_Backend->Name()});
    return true;
}

void AudioService::Shutdown()
{
    if (m_Backend)
    {
        m_Backend->Shutdown();
    }
}

void AudioService::PollBackendEvents()
{
    if (!m_Backend)
    {
        return;
    }

    BackendEvent event;
    while (m_Backend->PollEvents(event))
    {
        switch (event.kind)
        {
            case BackendEvent::Kind::StartOfFile:
            {
                m_CurrentUri = event.uri;
                flachead::events::Event ev;
                ev.type = flachead::events::Type::TrackChanged;
                ev.stringValue = event.uri;
                m_EventBus.Publish(ev);
                break;
            }
            case BackendEvent::Kind::FileLoaded:
            {
                m_CurrentUri = event.uri;
                flachead::events::Event ev;
                ev.type = flachead::events::Type::TrackLoaded;
                ev.stringValue = event.uri;
                m_EventBus.Publish(ev);
                break;
            }
            case BackendEvent::Kind::EndOfFile:
            {
                if (event.reason == kEndFileEof)
                {
                    m_EventBus.Publish(flachead::events::Type::PlaybackFinished);
                }
                else if (event.reason == kEndFileError)
                {
                    flachead::events::Event ev;
                    ev.type = flachead::events::Type::PlaybackError;
                    ev.stringValue = event.message.empty() ? "end of file error" : event.message;
                    m_EventBus.Publish(ev);
                }
                else
                {
                    m_EventBus.Publish(flachead::events::Type::PlaybackStopped);
                }
                break;
            }
            case BackendEvent::Kind::Error:
            {
                flachead::events::Event ev;
                ev.type = flachead::events::Type::PlaybackError;
                ev.stringValue = event.message.empty() ? "playback error" : event.message;
                m_EventBus.Publish(ev);
                break;
            }
            case BackendEvent::Kind::Idle:
            {
                m_EventBus.Publish(flachead::events::Type::PlaybackStopped);
                break;
            }
            default:
                break;
        }
    }
}

bool AudioService::PlayTrack(std::string_view uri)
{
    if (!m_Backend || !m_BackendAvailable)
    {
        return false;
    }
    m_CurrentUri = std::string{uri};
    if (!m_Backend->OpenMedia(uri))
    {
        flachead::core::Logger::Error("Failed to open media: " + std::string{uri});
        flachead::events::Event ev;
        ev.type = flachead::events::Type::PlaybackError;
        ev.stringValue = "cannot open file";
        m_EventBus.Publish(ev);
        return false;
    }
    m_Backend->Play();
    m_EventBus.Publish(flachead::events::Type::PlaybackStarted);
    return true;
}

bool AudioService::LoadTrack(std::string_view uri)
{
    if (!m_Backend || !m_BackendAvailable)
    {
        return false;
    }
    m_CurrentUri = std::string{uri};
    return m_Backend->OpenMedia(uri);
}

bool AudioService::QueueNext(std::string_view uri)
{
    if (!m_Backend || !m_BackendAvailable)
    {
        return false;
    }
    return m_Backend->QueueNext(uri);
}

void AudioService::ClearQueue()
{
    if (m_Backend)
    {
        m_Backend->ClearQueue();
    }
}

void AudioService::Play()
{
    if (m_Backend)
    {
        m_Backend->Play();
        m_EventBus.Publish(flachead::events::Type::PlaybackResumed);
    }
}

void AudioService::Pause()
{
    if (m_Backend)
    {
        m_Backend->Pause();
        m_EventBus.Publish(flachead::events::Type::PlaybackPaused);
    }
}

void AudioService::Toggle()
{
    if (IsPlaying())
    {
        Pause();
    }
    else
    {
        Play();
    }
}

void AudioService::Stop()
{
    if (m_Backend)
    {
        m_Backend->Stop();
        m_EventBus.Publish(flachead::events::Type::PlaybackStopped);
    }
}

void AudioService::Seek(double seconds)
{
    if (m_Backend)
    {
        m_Backend->Seek(seconds);
    }
}

void AudioService::SetVolume(float linear)
{
    m_Volume = std::clamp(linear, 0.0f, 1.0f);
    if (m_Backend)
    {
        m_Backend->SetVolume(m_Volume);
    }
    flachead::events::Event ev;
    ev.type = flachead::events::Type::VolumeChanged;
    ev.doubleValue = m_Volume;
    m_EventBus.Publish(ev);
}

double AudioService::GetPositionSeconds() const
{
    return m_Backend ? m_Backend->GetPositionSeconds() : 0.0;
}

double AudioService::GetDurationSeconds() const
{
    return m_Backend ? m_Backend->GetDurationSeconds() : 0.0;
}

bool AudioService::IsPlaying() const
{
    return m_Backend && m_Backend->IsPlaying();
}

bool AudioService::IsPaused() const
{
    return m_Backend && m_Backend->IsPaused();
}

std::string_view AudioService::BackendName() const
{
    return m_Backend ? m_Backend->Name() : "none";
}
} // namespace flachead::audio
