#include "InputManager.hpp"

namespace flachead::input
{
void InputManager::Bind(std::string_view, int)
{
}

bool InputManager::IsPressed(Action) const
{
    return false;
}
} // namespace flachead::input
