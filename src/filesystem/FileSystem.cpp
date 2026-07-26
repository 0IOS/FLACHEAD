#include "FileSystem.hpp"

#include <fstream>
#include <sstream>

namespace flachead::filesystem
{
bool FileSystem::Exists(std::string_view path) const
{
    std::ifstream stream(std::string{path});
    return stream.good();
}

bool FileSystem::IsDirectory(std::string_view path) const
{
    return false;
}

std::string FileSystem::ReadText(std::string_view path) const
{
    std::ifstream stream(std::string{path});
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}
} // namespace flachead::filesystem
