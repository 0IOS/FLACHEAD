#include "StorageManager.hpp"

#include <fstream>

namespace flachead::services
{
void StorageManager::Save(std::string_view key, std::string_view value)
{
    m_Dirty[std::string{key}] = std::string{value};
}

std::string StorageManager::Load(std::string_view key) const
{
    auto it = m_Dirty.find(key);
    if (it != m_Dirty.end())
    {
        return it->second;
    }

    std::ifstream stream(std::string{key});
    std::string result;
    std::getline(stream, result);
    return result;
}

bool StorageManager::Flush()
{
    bool success = true;

    for (const auto& [path, value] : m_Dirty)
    {
        std::ofstream stream(path);
        if (!stream)
        {
            success = false;
            continue;
        }
        stream << value;
    }

    if (success)
    {
        m_Dirty.clear();
    }
    return success;
}
} // namespace flachead::services
