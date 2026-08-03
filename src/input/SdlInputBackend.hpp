#pragma once

#include "InputBackend.hpp"

namespace flachead::input
{
// Desktop / touch-capable SDL backend. Pumps SDL's own event queue; emits
// every event unchanged. This is the default backend.
class SdlInputBackend final : public InputBackend
{
public:
    bool Initialize() override { return true; }
    void Shutdown() override {}

    void Poll(const EmitFn& emit) override
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            emit(event);
        }
    }

    const char* Name() const override { return "sdl"; }
};
} // namespace flachead::input
