#pragma once

#include <string>

namespace flachead::library
{
// Locates and caches cover art for songs and albums. Embedded artwork is
// extracted with ffmpeg; folder covers (cover.jpg, folder.png, ...) are copied
// into the art cache so the UI reads from one place. Everything here runs on
// the scan thread and must never throw.
class AlbumArtExtractor
{
public:
    // Default cache root: $HOME/.flachead/cache/art.
    static std::string DefaultCacheDir();

    // Returns the path of a conventional cover image in `folder`, or empty.
    static std::string FindCoverInFolder(const std::string& folder);

    // Extracts the embedded attached picture from `audioPath` into `outFile`
    // using ffmpeg. Returns false when the file has no usable artwork.
    static bool ExtractEmbeddedTo(const std::string& audioPath, const std::string& outFile);

    // Ensures artwork exists in `cacheDir` for the given song. Returns the
    // cached art path, or empty when no artwork is available. Embedded art is
    // preferred; otherwise a folder cover is copied in.
    static std::string EnsureCachedArt(const std::string& audioPath,
                                       const std::string& folder,
                                       bool hasEmbeddedArt,
                                       const std::string& cacheDir);

    // Creates a directory tree (mkdir -p). Returns false on failure.
    static bool MakeDirectoryTree(const std::string& path);
};
} // namespace flachead::library
