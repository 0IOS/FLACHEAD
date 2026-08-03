#pragma once

#include <string_view>

namespace flachead::audio
{
// Abstraction over the audio/video playback engine. The UI only talks to
// AudioService, which forwards to whatever backend is installed at boot.
//
// The production backend is libmpv (FLAC decode, gapless playback, video
// sync, subtitle rendering on a texture). A NullBackend satisfies this
// interface on hosts where libmpv is not linked yet, so the rest of the
// app stays buildable and runnable without it.
//
// Backend contract:
// - OpenMedia() replaces the current media; the backend must be idle after
//   it returns, and Play() starts decoding.
// - All methods must be callable from the main thread and must never block
//   longer than a few milliseconds; decoding/IO runs on the backend's own
//   threads. The render loop must never stall on a backend call.
// - Volume is linear 0.0..1.0, mapped to the backend's own scale.
class AudioBackend
{
public:
    virtual ~AudioBackend() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    virtual bool OpenMedia(std::string_view uri) = 0;
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void Seek(double seconds) = 0;
    virtual void SetVolume(float volume) = 0;

    virtual double GetPositionSeconds() const = 0;
    virtual bool IsPlaying() const = 0;
};
} // namespace flachead::audio
