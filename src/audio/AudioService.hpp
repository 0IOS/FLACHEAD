#pragma once

#include "AudioBackend.hpp"

#include <memory>

namespace flachead::audio
{
class AudioService
{
public:
    explicit AudioService(std::unique_ptr<AudioBackend> backend = nullptr);
    bool Initialize();
    void Shutdown();

private:
    std::unique_ptr<AudioBackend> m_Backend;
};
} // namespace flachead::audio
