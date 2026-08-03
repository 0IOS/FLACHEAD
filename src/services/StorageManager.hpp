#pragma once

#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace flachead::services
{
class StorageManager
{
public:
    void Save(std::string_view key, std::string_view value);
    std::string Load(std::string_view key) const;
    bool Flush();

private:
    std::map<std::string, std::string, std::less<>> m_Dirty;
};
} // namespace flachead::services
