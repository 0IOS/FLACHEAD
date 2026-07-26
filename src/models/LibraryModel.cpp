#include "LibraryModel.hpp"

namespace flachead::models
{
void LibraryModel::AddSong(const SongModel& song)
{
    m_Songs.push_back(song);
}

void LibraryModel::Clear()
{
    m_Songs.clear();
}

const std::vector<SongModel>& LibraryModel::Songs() const
{
    return m_Songs;
}
} // namespace flachead::models
