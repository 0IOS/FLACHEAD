#pragma once

#include <string>
#include <string_view>

namespace flachead::filesystem
{
class FileSystem
{
public:
    bool Exists(std::string_view path) const;
    bool IsDirectory(std::string_view path) const;
    std::string ReadText(std::string_view path) const;
};
} // namespace flachead::filesystem
