#pragma once

#include <cstdint>
#include <string>

namespace flachead::models
{
// A single audio track as stored in the library database and used by the
// playback pipeline. All fields are plain data; UI and playback read them
// through the owning service.
struct SongModel
{
    int64_t id{0};

    std::string title;
    std::string artist;
    std::string album;
    std::string albumArtist;
    std::string genre;
    std::string path;
    std::string folder;
    std::string codec;

    int  trackNo{0};
    int  discNo{0};
    int  year{0};
    int  channels{0};
    int  sampleRate{0};
    int  bitrate{0};

    double duration{0.0};

    int64_t fileSize{0};
    int64_t fileMtime{0};
    int64_t dateAdded{0};
    int64_t datePlayed{0};
    int     playCount{0};

    bool    favorite{false};
    bool    hasEmbeddedArt{false};
    std::string artPath;   // absolute path to cached cover art (empty = none)

    bool Valid() const { return !path.empty(); }

    // Display fallbacks for missing metadata.
    std::string DisplayTitle() const
    {
        return title.empty() ? (!path.empty() ? path.substr(path.find_last_of('/') + 1) : "Unknown") : title;
    }

    std::string DisplayArtist() const { return artist.empty() ? "Unknown Artist" : artist; }
    std::string DisplayAlbum() const { return album.empty() ? "Unknown Album" : album; }
};
} // namespace flachead::models
