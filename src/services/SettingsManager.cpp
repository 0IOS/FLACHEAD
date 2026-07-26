#include "SettingsManager.hpp"

namespace flachead::services
{
void SettingsManager::Set(std::string_view key, std::string_view value)
{
    m_Settings[std::string{key}] = std::string{value};
}

std::string SettingsManager::Get(std::string_view key) const
{
    auto it = m_Settings.find(std::string{key});
    if (it != m_Settings.end())
    {
        return it->second;
    }
    return {};
}
} // namespace flachead::services
