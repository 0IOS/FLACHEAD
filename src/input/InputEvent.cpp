#include "InputEvent.hpp"

namespace flachead::input
{
commands::Command KeyToCommand(SDL_Keycode key)
{
    switch (key)
    {
        case SDLK_ESCAPE: return commands::Command::Back;
        case SDLK_HOME:
        case SDLK_AC_HOME: return commands::Command::Home;
        case SDLK_UP: return commands::Command::Up;
        case SDLK_DOWN: return commands::Command::Down;
        case SDLK_LEFT: return commands::Command::Left;
        case SDLK_RIGHT: return commands::Command::Right;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE: return commands::Command::Select;
        case SDLK_MEDIA_PLAY_PAUSE: return commands::Command::PlayPause;
        case SDLK_MEDIA_NEXT_TRACK: return commands::Command::Next;
        case SDLK_MEDIA_PREVIOUS_TRACK: return commands::Command::Previous;
        case SDLK_P: return commands::Command::PlayPause;
        case SDLK_N: return commands::Command::Next;
        case SDLK_S: return commands::Command::ToggleShuffle;
        case SDLK_R: return commands::Command::ToggleRepeat;
        case SDLK_F: return commands::Command::ToggleFavorite;
        case SDLK_Q: return commands::Command::OpenQueue;        default: return commands::Command::None;
    }
}
} // namespace flachead::input
