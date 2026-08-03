#include "FFprobeExtractor.hpp"

#include "../core/Logger.hpp"
#include "Json.hpp"

#include <sys/stat.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace flachead::metadata
{
namespace
{
// Runs `ffprobe -v quiet -print_format json -show_format -show_streams <path>`
// and returns the raw stdout, or empty on failure.
std::string RunFFprobe(std::string_view path)
{
    const std::string command = "ffprobe -v quiet -print_format json -show_format -show_streams \"" + std::string{path} + "\" 2>/dev/null";

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        return {};
    }

    std::string output;
    char buffer[4096];
    std::size_t read;
    while ((read = fread(buffer, 1, sizeof(buffer), pipe)) > 0)
    {
        output.append(buffer, read);
    }

    const int rc = pclose(pipe);
    if (rc != 0)
    {
        flachead::core::Logger::Debug("ffprobe failed for " + std::string{path});
        return {};
    }

    return output;
}

int ParseTrackNumber(const std::string& value)
{
    // "3" or "3/12"
    const std::size_t slash = value.find('/');
    if (slash != std::string::npos)
    {
        return std::atoi(value.substr(0, slash).c_str());
    }
    return std::atoi(value.c_str());
}

int ParseYear(const std::string& value)
{
    // "2024", "2024-03-15", "2024-03-15T12:00:00"
    if (value.size() < 4)
    {
        return 0;
    }
    const int year = std::atoi(value.substr(0, 4).c_str());
    return year > 1900 && year < 2100 ? year : 0;
}

std::string TagString(const JsonValue& tags, const char* key)
{
    const JsonValue* value = tags.Find(key);
    if (value && value->GetType() == JsonValue::Type::String)
    {
        return value->AsString();
    }
    return {};
}

// ffprobe emits numbers as JSON numbers ("duration": 5.0) or quoted strings
// ("sample_rate": "44100"). Read both forms.
double TagNumber(const JsonValue& value, double fallback = 0.0)
{
    switch (value.GetType())
    {
        case JsonValue::Type::Number:
            return value.AsNumber();
        case JsonValue::Type::String:
            return std::strtod(value.AsString().c_str(), nullptr);
        default:
            return fallback;
    }
}

double TagNumber(const JsonValue& container, const char* key, double fallback = 0.0)
{
    const JsonValue* value = container.Find(key);
    return value ? TagNumber(*value, fallback) : fallback;
}
} // namespace

bool FFprobeExtractor::Available()
{
    return std::system("command -v ffprobe >/dev/null 2>&1") == 0;
}

bool FFprobeExtractor::Extract(std::string_view path, TrackMetadata& out)
{
    struct stat st{};
    if (stat(std::string{path}.c_str(), &st) != 0)
    {
        return false;
    }

    out = TrackMetadata{};
    out.fileSize = static_cast<std::int64_t>(st.st_size);
    out.fileMtime = static_cast<std::int64_t>(st.st_mtime);

    const std::string jsonText = RunFFprobe(path);
    if (jsonText.empty())
    {
        return false;
    }

    JsonValue root;
    if (!ParseJson(jsonText, root))
    {
        flachead::core::Logger::Warning("Failed to parse ffprobe output for " + std::string{path});
        return false;
    }

    const JsonValue* format = root.Find("format");
    if (format)
    {
        out.duration = TagNumber(*format, "duration", 0.0);
        out.bitrate = static_cast<int>(TagNumber(*format, "bit_rate", 0.0) / 1000.0);
        out.codec = format->GetString("format_long_name", format->GetString("format_name"));

        if (const JsonValue* tags = format->Find("tags"))
        {
            out.title = TagString(*tags, "title");
            out.artist = TagString(*tags, "artist");
            out.album = TagString(*tags, "album");
            out.albumArtist = TagString(*tags, "album_artist");
            if (out.albumArtist.empty())
            {
                out.albumArtist = TagString(*tags, "albumartist");
            }
            out.genre = TagString(*tags, "genre");

            const std::string track = TagString(*tags, "track");
            if (!track.empty())
            {
                out.trackNo = ParseTrackNumber(track);
            }
            const std::string disc = TagString(*tags, "disc");
            if (!disc.empty())
            {
                out.discNo = ParseTrackNumber(disc);
            }
            const std::string date = TagString(*tags, "date");
            if (!date.empty())
            {
                out.year = ParseYear(date);
            }
        }
    }

    const JsonValue* streams = root.Find("streams");
    if (streams && streams->GetType() == JsonValue::Type::Array)
    {
        for (const auto& stream : streams->AsArray())
        {
            const std::string codecType = stream.GetString("codec_type");
            if (codecType == "audio" && out.sampleRate == 0)
            {
                out.sampleRate = static_cast<int>(TagNumber(stream, "sample_rate", 0.0));
                out.channels = static_cast<int>(TagNumber(stream, "channels", 0.0));
                if (out.codec.empty() || out.codec == "unknown")
                {
                    out.codec = stream.GetString("codec_name");
                }
                if (out.duration <= 0.0)
                {
                    out.duration = TagNumber(stream, "duration", 0.0);
                }
            }
            else if (codecType == "video")
            {
                const std::string codecName = stream.GetString("codec_name");
                if (codecName == "mjpeg" || codecName == "png" || codecName == "bmp" || codecName == "gif")
                {
                    out.hasEmbeddedArt = true;
                }
            }
        }
    }

    return true;
}
} // namespace flachead::metadata
