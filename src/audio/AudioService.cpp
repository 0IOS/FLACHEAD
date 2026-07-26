#include "AudioService.hpp"

namespace flachead::audio
{
AudioService::AudioService(std::unique_ptr<AudioBackend> backend)
    : m_Backend(std::move(backend))
{
}

bool AudioService::Initialize()
{
    if (m_Backend)
    {
        return m_Backend->Initialize();
    }
    return true;
}

void AudioService::Shutdown()
{
    if (m_Backend)
    {
        m_Backend->Shutdown();
    }
}
} // namespace flachead::audio
