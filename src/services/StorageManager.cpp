#include "StorageManager.hpp"

#include <fstream>

namespace flachead::services
{
bool StorageManager::Save(std::string_view key, std::string_view value) const
{
    std::ofstream stream(std::string{key});
    if (!stream)
    {
        return false;
    }

    stream << value;
    return true;
}

std::string StorageManager::Load(std::string_view key) const
{
    std::ifstream stream(std::string{key});
    std::string result;
    std::getline(stream, result);
    return result;
}
} // namespace flachead::services
