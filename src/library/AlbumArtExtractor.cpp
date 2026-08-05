#include "AlbumArtExtractor.hpp"

#include "../core/Logger.hpp"
#include "../core/PathUtils.hpp"

#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace flachead::library
{
namespace
{
bool FileExists(const std::string& path)
{
    struct stat st {};
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

// Runs a shell command that is expected to succeed; logs on failure.
bool RunCommand(const std::string& command)
{
    const int rc = std::system(command.c_str());
    if (rc != 0)
    {
        flachead::core::Logger::Debug("command failed (" + std::to_string(rc) + "): " + command);
        return false;
    }
    return true;
}

std::string CacheKey(const std::string& album, const std::string& artist)
{
    return std::to_string(flachead::core::path::Hash(album + "\x1f" + artist));
}
} // namespace

std::string AlbumArtExtractor::DefaultCacheDir()
{
    const char* home = std::getenv("HOME");
    return std::string{home ? home : "/tmp"} + "/.flachead/cache/art";
}

bool AlbumArtExtractor::MakeDirectoryTree(const std::string& path)
{
    if (path.empty())
    {
        return false;
    }
    std::string current;
    for (const char c : path)
    {
        current.push_back(c);
        if (c == '/' && current.size() > 1)
        {
            ::mkdir(current.c_str(), 0755);
        }
    }
    ::mkdir(path.c_str(), 0755);
    return FileExists(path);
}

bool AlbumArtExtractor::ExtractEmbeddedTo(const std::string& audioPath, const std::string& outFile)
{
    // ffmpeg -y -i <audio> -map 0:v:0 -c:v copy <out>
    const std::string command =
        "ffmpeg -y -loglevel error -i \"" + audioPath + "\" -map 0:v:0 -c:v copy \"" + outFile +
        "\"";
    if (!RunCommand(command))
    {
        return false;
    }
    return FileExists(outFile);
}

std::string AlbumArtExtractor::FindCoverInFolder(const std::string& folder)
{
    static const char* kCoverNames[] = {
        "cover.jpg", "cover.jpeg", "cover.png", "folder.jpg", "folder.jpeg",
        "folder.png", "front.jpg", "front.png", "album.jpg", "album.png",
    };

    for (const char* name : kCoverNames)
    {
        const std::string candidate = folder + "/" + name;
        if (FileExists(candidate))
        {
            return candidate;
        }
    }
    return {};
}

std::string AlbumArtExtractor::EnsureCachedArt(const std::string& audioPath,
                                               const std::string& folder,
                                               bool hasEmbeddedArt,
                                               const std::string& cacheDir)
{
    MakeDirectoryTree(cacheDir);

    const std::string key = CacheKey(folder, "");
    const std::string cachedJpg = cacheDir + "/" + key + ".jpg";

    if (hasEmbeddedArt && !FileExists(cachedJpg))
    {
        if (ExtractEmbeddedTo(audioPath, cachedJpg))
        {
            return cachedJpg;
        }
    }

    const std::string cover = FindCoverInFolder(folder);
    if (!cover.empty())
    {
        const std::string ext = flachead::core::path::Extension(cover);
        const std::string target = cacheDir + "/" + key + (ext.empty() ? ".jpg" : ext);
        if (FileExists(target))
        {
            return target;
        }
        // Copy rather than move: the folder cover belongs to the user's library.
        const std::string command = "cp \"" + cover + "\" \"" + target + "\"";
        if (RunCommand(command) && FileExists(target))
        {
            return target;
        }
    }

    return {};
}
} // namespace flachead::library
