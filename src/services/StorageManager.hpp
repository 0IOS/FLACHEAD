#pragma once

#include <string>
#include <string_view>

namespace flachead::services
{
class StorageManager
{
public:
    bool Save(std::string_view key, std::string_view value) const;
    std::string Load(std::string_view key) const;
};
} // namespace flachead::services
