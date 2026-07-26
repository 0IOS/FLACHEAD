#pragma once

#include <string>
#include <string_view>

namespace flachead::models
{
class AlbumModel
{
public:
    AlbumModel() = default;
    AlbumModel(std::string_view name, std::string_view artist, std::string_view artworkPath);

    void SetName(std::string_view name);
    void SetArtist(std::string_view artist);
    void SetArtworkPath(std::string_view artworkPath);

    std::string_view Name() const;
    std::string_view Artist() const;
    std::string_view ArtworkPath() const;

private:
    std::string m_Name;
    std::string m_Artist;
    std::string m_ArtworkPath;
};
} // namespace flachead::models
