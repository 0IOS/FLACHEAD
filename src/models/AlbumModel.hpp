#pragma once

#include <cstdint>
#include <string>

namespace flachead::models
{
struct AlbumModel
{
    int64_t id{0};
    std::string name;
    std::string artist;
    std::string artPath;
    int trackCount{0};
    int year{0};

    std::string DisplayName() const { return name.empty() ? "Unknown Album" : name; }
};
} // namespace flachead::models
