#include "AudioService.hpp"

namespace flachead::audio
{
AudioService::AudioService(std::unique_ptr<AudioBackend> backend)
    : m_Backend(std::move(backend))
{
}

bool AudioService::Initialize()
{
    return m_Backend ? m_Backend->Initialize() : true;
}

void AudioService::Shutdown()
{
    if (m_Backend)
    {
        m_Backend->Shutdown();
    }
}

bool AudioService::OpenMedia(std::string_view uri)
{
    return m_Backend ? m_Backend->OpenMedia(uri) : false;
}

void AudioService::Play()
{
    if (m_Backend)
    {
        m_Backend->Play();
    }
}

void AudioService::Pause()
{
    if (m_Backend)
    {
        m_Backend->Pause();
    }
}

void AudioService::Stop()
{
    if (m_Backend)
    {
        m_Backend->Stop();
    }
}

void AudioService::Seek(double seconds)
{
    if (m_Backend)
    {
        m_Backend->Seek(seconds);
    }
}

void AudioService::SetVolume(float volume)
{
    if (m_Backend)
    {
        m_Backend->SetVolume(volume);
    }
}

double AudioService::GetPositionSeconds() const
{
    return m_Backend ? m_Backend->GetPositionSeconds() : 0.0;
}

bool AudioService::IsPlaying() const
{
    return m_Backend ? m_Backend->IsPlaying() : false;
}
} // namespace flachead::audio
