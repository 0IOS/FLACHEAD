#pragma once

namespace flachead::commands
{
// Every user action in the operating environment is a Command. Input, buttons
// and gestures translate into commands; the CommandCenter routes them to the
// focused screen, the app shell, or the system.
enum class Command
{
    None = 0,

    // Navigation / shell
    Back,
    Home,
    Launcher,
    TaskOverview,

    // Directional
    Up,
    Down,
    Left,
    Right,
    Select,

    // Playback
    Play,
    Pause,
    PlayPause,
    Stop,
    Next,
    Previous,
    SeekForward,
    SeekBack,
    VolumeUp,
    VolumeDown,
    ToggleShuffle,
    ToggleRepeat,
    ToggleFavorite,

    // Apps / actions
    LaunchApp,
    OpenSettings,
    OpenSearch,
    OpenQueue,
    Shutdown,
    Restart,
    Sleep,
    Wake,
    Refresh,
};

const char* CommandName(Command command);
} // namespace flachead::commands
