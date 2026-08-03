#pragma once

#include "AudioBackend.hpp"
#include "NullBackend.hpp"

#include <memory>
#include <string_view>

namespace flachead::audio
{
class AudioService
{
public:
    explicit AudioService(std::unique_ptr<AudioBackend> backend = std::make_unique<NullBackend>());
    bool Initialize();
    void Shutdown();

    bool OpenMedia(std::string_view uri);
    void Play();
    void Pause();
    void Stop();
    void Seek(double seconds);
    void SetVolume(float volume);
    double GetPositionSeconds() const;
    bool IsPlaying() const;

private:
    std::unique_ptr<AudioBackend> m_Backend;
};
} // namespace flachead::audio
