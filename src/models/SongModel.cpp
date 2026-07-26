#include "SongModel.hpp"

namespace flachead::models
{
SongModel::SongModel(std::string_view title, std::string_view artist, std::string_view album, std::string_view path)
    : m_Title(title),
      m_Artist(artist),
      m_Album(album),
      m_Path(path)
{
}

void SongModel::SetTitle(std::string_view title)
{
    m_Title = title;
}

void SongModel::SetArtist(std::string_view artist)
{
    m_Artist = artist;
}

void SongModel::SetAlbum(std::string_view album)
{
    m_Album = album;
}

void SongModel::SetPath(std::string_view path)
{
    m_Path = path;
}

std::string_view SongModel::Title() const
{
    return m_Title;
}

std::string_view SongModel::Artist() const
{
    return m_Artist;
}

std::string_view SongModel::Album() const
{
    return m_Album;
}

std::string_view SongModel::Path() const
{
    return m_Path;
}
} // namespace flachead::models
