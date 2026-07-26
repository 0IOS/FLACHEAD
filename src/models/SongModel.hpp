#pragma once

#include <string>
#include <string_view>

namespace flachead::models
{
class SongModel
{
public:
    SongModel() = default;
    SongModel(std::string_view title,
              std::string_view artist,
              std::string_view album,
              std::string_view path);

    void SetTitle(std::string_view title);
    void SetArtist(std::string_view artist);
    void SetAlbum(std::string_view album);
    void SetPath(std::string_view path);

    std::string_view Title() const;
    std::string_view Artist() const;
    std::string_view Album() const;
    std::string_view Path() const;

private:
    std::string m_Title;
    std::string m_Artist;
    std::string m_Album;
    std::string m_Path;
};
} // namespace flachead::models
