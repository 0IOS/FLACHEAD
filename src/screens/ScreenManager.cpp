#include "ScreenManager.hpp"

#include "../core/Logger.hpp"

#include <stdexcept>
#include <vector>

namespace flachead::screens
{
void ScreenManager::Register(std::string name, std::unique_ptr<Screen> screen)
{
    m_Screens.emplace(std::move(name), std::move(screen));
}

void ScreenManager::Push(std::string_view name)
{
    auto it = m_Screens.find(std::string{name});
    if (it == m_Screens.end())
    {
        throw std::runtime_error("Unknown screen requested");
    }

    m_Stack.push_back(std::string{name});
    it->second->OnEnter();
}

void ScreenManager::Pop()
{
    if (m_Stack.empty())
    {
        return;
    }

    auto& screen = m_Screens.at(m_Stack.back());
    screen->OnExit();
    m_Stack.pop_back();
}

void ScreenManager::Clear()
{
    while (!m_Stack.empty())
    {
        Pop();
    }
}

Screen* ScreenManager::Current() const
{
    if (m_Stack.empty())
    {
        return nullptr;
    }

    return m_Screens.at(m_Stack.back()).get();
}
} // namespace flachead::screens
