#pragma once

#include "AudioBackend.hpp"

namespace flachead::audio
{
// No-op backend used until the libmpv backend lands. Keeps the app fully
// buildable and runnable on hosts without an audio library.
class NullBackend final : public AudioBackend
{
public:
    bool Initialize() override { return true; }
    void Shutdown() override {}

    bool OpenMedia(std::string_view) override { return true; }
    void Play() override {}
    void Pause() override {}
    void Stop() override {}
    void Seek(double) override {}
    void SetVolume(float) override {}

    double GetPositionSeconds() const override { return 0.0; }
    bool IsPlaying() const override { return false; }
};
} // namespace flachead::audio
