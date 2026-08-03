#include "PathUtils.hpp"

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <unordered_set>

namespace flachead::core
{
namespace path
{
namespace
{
constexpr const char* kAudioExtensions[] = {
    ".flac", ".mp3", ".ogg", ".oga", ".m4a", ".aac", ".wav", ".opus",
};

constexpr const char* kImageExtensions[] = {
    ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp",
};

const std::unordered_set<std::string>& AudioSet()
{
    static const std::unordered_set<std::string> set{
        std::begin(kAudioExtensions), std::end(kAudioExtensions)};
    return set;
}

const std::unordered_set<std::string>& ImageSet()
{
    static const std::unordered_set<std::string> set{
        std::begin(kImageExtensions), std::end(kImageExtensions)};
    return set;
}
} // namespace

std::string Extension(std::string_view path)
{
    const std::size_t dot = path.find_last_of('.');
    const std::size_t slash = path.find_last_of("/\\");
    if (dot == std::string_view::npos || (slash != std::string_view::npos && dot < slash))
    {
        return {};
    }

    std::string extension{path.substr(dot)};
    for (char& c : extension)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return extension;
}

std::string BaseName(std::string_view path)
{
    const std::size_t slash = path.find_last_of("/\\");
    if (slash == std::string_view::npos)
    {
        return std::string{path};
    }
    return std::string{path.substr(slash + 1)};
}

std::string Stem(std::string_view path)
{
    const std::string base = BaseName(path);
    const std::size_t dot = base.find_last_of('.');
    if (dot == std::string::npos)
    {
        return base;
    }
    return base.substr(0, dot);
}

std::string ParentDirectory(std::string_view path)
{
    const std::size_t slash = path.find_last_of("/\\");
    if (slash == std::string_view::npos)
    {
        return ".";
    }
    if (slash == 0)
    {
        return "/";
    }
    return std::string{path.substr(0, slash)};
}

std::string FolderName(std::string_view path)
{
    const std::string parent = ParentDirectory(path);
    return BaseName(parent);
}

bool IsAudioFile(std::string_view path)
{
    return AudioSet().count(Extension(path)) > 0;
}

bool IsImageFile(std::string_view path)
{
    return ImageSet().count(Extension(path)) > 0;
}

std::size_t CollectFiles(std::string_view root, std::vector<std::string>& out)
{
    std::error_code ec;
    const std::filesystem::path base{root};
    if (!std::filesystem::exists(base, ec) || !std::filesystem::is_directory(base, ec))
    {
        return 0;
    }

    std::size_t count = 0;
    std::filesystem::recursive_directory_iterator it(
        base, std::filesystem::directory_options::skip_permission_denied, ec);
    const std::filesystem::recursive_directory_iterator end;

    for (; it != end; it.increment(ec))
    {
        if (ec)
        {
            it.disable_recursion_pending();
            ec.clear();
            continue;
        }

        const std::string name = it->path().filename().string();
        if (it->is_directory())
        {
            if (!name.empty() && name.front() == '.')
            {
                it.disable_recursion_pending();
            }
            continue;
        }

        if (!it->is_regular_file(ec))
        {
            continue;
        }

        if (IsAudioFile(it->path().string()))
        {
            out.push_back(it->path().string());
            ++count;
        }
    }
    return count;
}

std::string SanitizeSegment(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    for (const char c : text)
    {
        if (c == '/' || c == '\\' || c == '\0' || c == ':' || c == '*')
        {
            result.push_back('_');
        }
        else
        {
            result.push_back(c);
        }
    }
    return result;
}

std::uint64_t Hash(std::string_view text)
{
    std::uint64_t hash = 14695981039346656037ULL;
    for (const unsigned char c : text)
    {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}
} // namespace path
} // namespace flachead::core
