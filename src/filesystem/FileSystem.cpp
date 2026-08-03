#include "FileSystem.hpp"

#include <filesystem>
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
    std::error_code ec;
    return std::filesystem::is_directory(std::filesystem::path{path}, ec);
}

std::string FileSystem::ReadText(std::string_view path) const
{
    std::ifstream stream(std::string{path});
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}
} // namespace flachead::filesystem
