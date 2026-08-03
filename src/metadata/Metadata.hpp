#pragma once

#include <cstdint>
#include <string>

namespace flachead::metadata
{
// Extracted audio file metadata, independent of any tagging library.
struct TrackMetadata
{
    std::string title;
    std::string artist;
    std::string album;
    std::string albumArtist;
    std::string genre;
    std::string codec;
    std::string folder;

    int trackNo{0};
    int discNo{0};
    int year{0};
    int channels{0};
    int sampleRate{0};
    int bitrate{0};

    double duration{0.0};

    std::int64_t fileSize{0};
    std::int64_t fileMtime{0};

    bool hasEmbeddedArt{false};
};

// Common interface for metadata extraction backends. Implementations are
// cheap to construct and must never throw.
class MetadataExtractor
{
public:
    virtual ~MetadataExtractor() = default;

    // Fills `out` with metadata for `path`. Returns false when the file
    // cannot be read or has no usable metadata.
    virtual bool Extract(std::string_view path, TrackMetadata& out) = 0;

    // Human readable backend name for diagnostics.
    virtual std::string_view Name() const = 0;
};
} // namespace flachead::metadata
