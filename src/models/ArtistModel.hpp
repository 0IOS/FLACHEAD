#pragma once

#include <cstdint>
#include <string>

namespace flachead::models
{
struct ArtistModel
{
    int64_t id{0};
    std::string name;
    int albumCount{0};
    int trackCount{0};
};
} // namespace flachead::models
