#include "PlaylistModel.hpp"

namespace flachead::models
{
PlaylistModel::PlaylistModel(std::string_view name)
    : m_Name(name)
{
}

void PlaylistModel::SetName(std::string_view name)
{
    m_Name = name;
}

void PlaylistModel::AddTrack(std::string_view path)
{
    m_Tracks.emplace_back(path);
}

std::string_view PlaylistModel::Name() const
{
    return m_Name;
}

const std::vector<std::string>& PlaylistModel::Tracks() const
{
    return m_Tracks;
}
} // namespace flachead::models
