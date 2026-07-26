#pragma once

namespace flachead::audio
{
class AudioBackend
{
public:
    virtual ~AudioBackend() = default;
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
};
} // namespace flachead::audio
