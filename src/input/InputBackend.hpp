#pragma once

#include <SDL3/SDL.h>

#include <functional>

namespace flachead::input
{
// Source of SDL_Event objects. The rest of the app consumes SDL_Event (screens
// implement HandleEvent), so every input source — keyboard, mouse, touch, GPIO
// buttons, rotary encoder — produces SDL_Event and nothing else needs to know
// where input came from.
//
// Contract:
// - Poll() must not block; it is called once per frame from the main loop.
// - Events are delivered through `emit` immediately (SDL_PushEvent is not used
//   so ordering with other SDL events is preserved by the caller).
// - The backend must stay responsive under load: never sleep in Poll().
class InputBackend
{
public:
    using EmitFn = std::function<void(const SDL_Event&)>;

    virtual ~InputBackend() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Poll(const EmitFn& emit) = 0;
    virtual const char* Name() const = 0;
};
} // namespace flachead::input
