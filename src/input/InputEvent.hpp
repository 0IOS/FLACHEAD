#pragma once

#include "../commands/Command.hpp"
#include "../math/Vec2.hpp"

#include <SDL3/SDL_keycode.h>

#include <cstdint>

namespace flachead::input
{
enum class InputSource
{
    Keyboard,
    Touch,
    Mouse,
    Gpio,
};

enum class InputAction
{
    KeyDown,
    KeyUp,
    Press,
    Release,
    Tap,
    DoubleTap,
    Hold,
    Swipe,
    DragMove,
    DragEnd,
};

// Semantic input event produced by the InputManager from raw SDL events. The
// UI consumes these; it never sees SDL_Event directly.
struct InputEvent
{
    InputAction action{InputAction::Tap};
    InputSource source{InputSource::Touch};
    SDL_Keycode key{0};
    Vec2 position;
    Vec2 delta;
    float velocity{0.0f};
    int tapCount{0};
};

// Converts a key event into a command, or Command::None when the key is not a
// global command. The current screen may still handle the raw key first.
commands::Command KeyToCommand(SDL_Keycode key);
} // namespace flachead::input
