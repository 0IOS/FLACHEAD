#include "ScreenManager.hpp"

#include "../core/Logger.hpp"

#include <stdexcept>
#include <vector>

namespace flachead::screens
{
void ScreenManager::RegisterFactory(std::string name, Factory factory)
{
    m_Factories.emplace(std::move(name), std::move(factory));
}

void ScreenManager::Push(std::string_view name)
{
    const std::string key{name};

    auto factoryIt = m_Factories.find(key);
    if (factoryIt == m_Factories.end())
    {
        throw std::runtime_error("Unknown screen requested");
    }

    auto screenIt = m_Screens.find(key);
    if (screenIt == m_Screens.end())
    {
        auto created = factoryIt->second();
        screenIt = m_Screens.emplace(key, std::move(created)).first;
    }

    m_Stack.push_back(key);
    screenIt->second->OnEnter();
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

void ScreenManager::PopTo(std::string_view name)
{
    const std::string key{name};
    while (!m_Stack.empty() && m_Stack.back() != key)
    {
        Pop();
    }
}

void ScreenManager::Clear()
{
    while (!m_Stack.empty())
    {
        Pop();
    }
}

void ScreenManager::Update(float deltaSeconds)
{
    if (auto* screen = Current())
    {
        screen->OnUpdate(deltaSeconds);
    }
}

void ScreenManager::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    if (auto* screen = Current())
    {
        screen->Render(canvas, width, height);
    }
}

bool ScreenManager::NeedsRender() const
{
    if (auto* screen = Current())
    {
        return screen->NeedsRender();
    }
    return false;
}

bool ScreenManager::IsOnStack(std::string_view name) const
{
    const std::string key{name};
    for (const auto& entry : m_Stack)
    {
        if (entry == key)
        {
            return true;
        }
    }
    return false;
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
