#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace flachead::services
{
class SettingsManager
{
public:
    void Set(std::string_view key, std::string_view value);
    std::string Get(std::string_view key) const;

private:
    std::unordered_map<std::string, std::string> m_Settings;
};
} // namespace flachead::services
