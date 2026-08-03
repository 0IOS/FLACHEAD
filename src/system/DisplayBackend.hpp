#pragma once

namespace flachead::system
{
// Final stage that receives rendered frames. The SDL implementation presents
// to a window; a future SPI/DRM implementation (Raspberry Pi Zero W panel) will
// write the software-rendered framebuffer to the display device instead.
//
// Contract:
// - Create() acquires the output; Destroy() releases it. Present() commits the
//   current frame and must return quickly (never block on the panel).
// - Size/geometry stays owned by the window/system layer, not this class.
class DisplayBackend
{
public:
    virtual ~DisplayBackend() = default;

    virtual bool Create() = 0;
    virtual void Destroy() = 0;
    virtual void Present() = 0;
    virtual const char* Name() const = 0;
};
} // namespace flachead::system
