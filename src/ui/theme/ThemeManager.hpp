#pragma once

#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace flachead::theme
{
struct ThemeDefinition
{
    std::string name;
    std::map<std::string, std::string, std::less<>> values;
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
