#pragma once

#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace flachead::services
{
class SettingsManager
{
public:
    void Set(std::string_view key, std::string_view value);
    std::string Get(std::string_view key) const;

private:
    std::map<std::string, std::string, std::less<>> m_Settings;
};
} // namespace flachead::services
