#pragma once

#include "DisplayBackend.hpp"

#include <SDL3/SDL.h>

namespace flachead::system
{
// Presents to an SDL renderer / window. The default backend; on the Pi this
// is swapped for a panel backend once the SPI/DRM path lands.
class SdlDisplayBackend final : public DisplayBackend
{
public:
    explicit SdlDisplayBackend(SDL_Renderer* renderer)
        : m_Renderer(renderer)
    {
    }

    bool Create() override { return m_Renderer != nullptr; }
    void Destroy() override {}

    void Present() override
    {
        if (m_Renderer)
        {
            SDL_RenderPresent(m_Renderer);
        }
    }

    const char* Name() const override { return "sdl"; }

private:
    SDL_Renderer* m_Renderer;
};
} // namespace flachead::system
