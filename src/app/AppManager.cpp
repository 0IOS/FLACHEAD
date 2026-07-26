#include "AppManager.hpp"

#include "../core/Logger.hpp"

namespace flachead::app
{
AppManager::AppManager()
    : m_ActiveApplication("launcher")
{
}

bool AppManager::Initialize()
{
    m_Initialized = true;
    flachead::core::Logger::Info("AppManager initialized");
    return true;
}

void AppManager::Shutdown()
{
    m_Initialized = false;
    flachead::core::Logger::Info("AppManager shutdown");
}

void AppManager::Tick(float)
{
    if (!m_Initialized)
    {
        return;
    }
}

std::string_view AppManager::ActiveApplication() const
{
    return m_ActiveApplication;
}

void AppManager::SetActiveApplication(std::string_view name)
{
    m_ActiveApplication = name;
}
} // namespace flachead::app
