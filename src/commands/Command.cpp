#include "Command.hpp"

namespace flachead::commands
{
const char* CommandName(Command command)
{
    switch (command)
    {
        case Command::None: return "None";
        case Command::Back: return "Back";
        case Command::Home: return "Home";
        case Command::Launcher: return "Launcher";
        case Command::TaskOverview: return "TaskOverview";
        case Command::Up: return "Up";
        case Command::Down: return "Down";
        case Command::Left: return "Left";
        case Command::Right: return "Right";
        case Command::Select: return "Select";
        case Command::Play: return "Play";
        case Command::Pause: return "Pause";
        case Command::PlayPause: return "PlayPause";
        case Command::Stop: return "Stop";
        case Command::Next: return "Next";
        case Command::Previous: return "Previous";
        case Command::SeekForward: return "SeekForward";
        case Command::SeekBack: return "SeekBack";
        case Command::VolumeUp: return "VolumeUp";
        case Command::VolumeDown: return "VolumeDown";
        case Command::ToggleShuffle: return "ToggleShuffle";
        case Command::ToggleRepeat: return "ToggleRepeat";
        case Command::ToggleFavorite: return "ToggleFavorite";
        case Command::LaunchApp: return "LaunchApp";
        case Command::OpenSettings: return "OpenSettings";
        case Command::OpenSearch: return "OpenSearch";
        case Command::OpenQueue: return "OpenQueue";
        case Command::Shutdown: return "Shutdown";
        case Command::Restart: return "Restart";
        case Command::Sleep: return "Sleep";
        case Command::Wake: return "Wake";
        case Command::Refresh: return "Refresh";
    }
    return "None";
}
} // namespace flachead::commands
