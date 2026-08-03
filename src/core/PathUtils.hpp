#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace flachead::core
{
namespace path
{
// Lowercase file extension including the dot: "song.flac" -> ".flac".
std::string Extension(std::string_view path);

// File name with extension: "/a/b/song.flac" -> "song.flac".
std::string BaseName(std::string_view path);

// File name without extension: "/a/b/song.flac" -> "song".
std::string Stem(std::string_view path);

// Parent directory: "/a/b/song.flac" -> "/a/b".
std::string ParentDirectory(std::string_view path);

// The immediate folder name the file lives in: "/a/b/song.flac" -> "b".
std::string FolderName(std::string_view path);

// True when the extension matches one of the supported audio formats.
bool IsAudioFile(std::string_view path);

// True when the file looks like a cover image (folder.jpg, cover.png, ...).
bool IsImageFile(std::string_view path);

// Recursively collect all regular files under root into `out`, returning the
// number of files found. Skips hidden directories. Follows no symlinks.
std::size_t CollectFiles(std::string_view root, std::vector<std::string>& out);

// Sanitize a string for use as a single path segment (no slashes, nulls).
std::string SanitizeSegment(std::string_view text);

// 64-bit hash of a string (FNV-1a), used for cache keys.
std::uint64_t Hash(std::string_view text);
} // namespace path
} // namespace flachead::core
