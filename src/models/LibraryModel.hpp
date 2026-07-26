#pragma once

#include "SongModel.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace flachead::models
{
class LibraryModel
{
public:
    void AddSong(const SongModel& song);
    void Clear();

    const std::vector<SongModel>& Songs() const;

private:
    std::vector<SongModel> m_Songs;
};
} // namespace flachead::models
