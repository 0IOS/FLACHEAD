#include "AlbumModel.hpp"

namespace flachead::models
{
AlbumModel::AlbumModel(std::string_view name, std::string_view artist, std::string_view artworkPath)
    : m_Name(name),
      m_Artist(artist),
      m_ArtworkPath(artworkPath)
{
}

void AlbumModel::SetName(std::string_view name)
{
    m_Name = name;
}

void AlbumModel::SetArtist(std::string_view artist)
{
    m_Artist = artist;
}

void AlbumModel::SetArtworkPath(std::string_view artworkPath)
{
    m_ArtworkPath = artworkPath;
}

std::string_view AlbumModel::Name() const
{
    return m_Name;
}

std::string_view AlbumModel::Artist() const
{
    return m_Artist;
}

std::string_view AlbumModel::ArtworkPath() const
{
    return m_ArtworkPath;
}
} // namespace flachead::models
