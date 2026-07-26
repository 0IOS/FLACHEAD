#pragma once

#include <string_view>

namespace flachead::input
{
enum class Action
{
    NavigateUp,
    NavigateDown,
    NavigateLeft,
    NavigateRight,
    Accept,
    Back,
    PlayPause,
    NextTrack,
    PreviousTrack,
    VolumeUp,
    VolumeDown,
    TouchTap,
    TouchDrag,
    TouchScroll,
    Mouse,
    Keyboard,
    Gamepad,
    GpioButton,
    RotaryEncoder
};

class InputManager
{
public:
    void Bind(std::string_view action, int code);
    bool IsPressed(Action action) const;

private:
    int m_Binding{0};
};
} // namespace flachead::input
