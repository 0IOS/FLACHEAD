#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace flachead::theme
{
struct ThemeDefinition
{
    std::string name;
    std::unordered_map<std::string, std::string> values;
};

class ThemeManager
{
public:
    ThemeManager() = default;

    void Load(std::string_view name);
    std::string_view Get(std::string_view key) const;

private:
    ThemeDefinition m_CurrentTheme;
};
} // namespace flachead::theme
