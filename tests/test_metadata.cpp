#include "../src/metadata/FFprobeExtractor.hpp"
#include "../src/metadata/Json.hpp"
#include "../src/metadata/Metadata.hpp"
#include "test_util.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

using flachead::metadata::FFprobeExtractor;
using flachead::metadata::JsonValue;
using flachead::metadata::ParseJson;
using flachead::metadata::TrackMetadata;

static bool g_HasFFprobe = false;

static void EnsureTestFixture()
{
    if (std::FILE* file = std::fopen("/tmp/test_song.flac", "rb"))
    {
        std::fclose(file);
        return;
    }
    // Regenerate the 5-second tagged FLAC the extractor tests rely on.
    const int rc = std::system(
        "ffmpeg -y -f lavfi -i sine=frequency=440:duration=5 -c:a flac "
        "-metadata title=\"Test Song\" -metadata artist=\"Test Artist\" "
        "-metadata album=\"Test Album\" -metadata genre=\"Rock\" "
        "-metadata track=\"3/10\" -metadata date=\"2023\" "
        "/tmp/test_song.flac > /dev/null 2>&1");
    (void)rc;
}

int main()
{
    EnsureTestFixture();

    RunTest("json parser", [&] {
        JsonValue root;
        const char* text = R"({"title":"A","n":42,"pi":3.5,"ok":true,"arr":[1,"x",null],"obj":{"k":"v"},"uni":"\u00e9"})";
        Check(ParseJson(text, root), "parse object");
        Check(root.GetString("title") == "A", "string value");
        Check(std::abs(root.GetNumber("n") - 42.0) < 0.001, "int number");
        Check(std::abs(root.GetNumber("pi") - 3.5) < 0.001, "float number");
        Check(root.GetBool("ok"), "bool value");

        const JsonValue* arr = root.Find("arr");
        Check(arr && arr->GetType() == JsonValue::Type::Array, "array present");
        if (arr && arr->GetType() == JsonValue::Type::Array)
        {
            Check(arr->AsArray().size() == 3, "array size");
            Check(arr->AsArray()[0].AsNumber() == 1.0, "array element number");
            Check(arr->AsArray()[1].AsString() == "x", "array element string");
        }
        Check(root.Find("missing") == nullptr, "missing key");

        JsonValue nested;
        Check(ParseJson(R"({"outer":{"inner":[true,false]}})", nested), "nested object");
        Check(nested.GetBool("outer") == false, "no direct bool for object");

        JsonValue invalid;
        Check(!ParseJson("{broken", invalid), "reject malformed json");
        Check(!ParseJson("[1,2,]", invalid), "reject trailing comma");
    });

    g_HasFFprobe = FFprobeExtractor::Available();
    RunTest("ffprobe extractor", [&] {
        Check(g_HasFFprobe, "ffprobe available");
        if (!g_HasFFprobe)
        {
            return;
        }

        TrackMetadata meta;
        const bool ok = FFprobeExtractor{}.Extract("/tmp/test_song.flac", meta);
        Check(ok, "extract flac file");
        if (!ok)
        {
            return;
        }
        Check(meta.title == "Test Song", "title: " + meta.title);
        Check(meta.artist == "Test Artist", "artist: " + meta.artist);
        Check(meta.album == "Test Album", "album: " + meta.album);
        Check(meta.genre == "Rock", "genre: " + meta.genre);
        Check(meta.trackNo == 3, "track number");
        Check(meta.year == 2023, "year");
        Check(std::abs(meta.duration - 5.0) < 0.5, "duration ~5s");
        Check(meta.sampleRate == 44100, "sample rate");
        auto lower = [](std::string s) {
            for (char& c : s)
            {
                if (c >= 'A' && c <= 'Z')
                    c = static_cast<char>(c + 32);
            }
            return s;
        };
        Check(lower(meta.codec).find("flac") != std::string::npos, "codec contains flac: " + meta.codec);
        Check(meta.fileSize > 0, "file size captured");

        TrackMetadata bad;
        Check(!FFprobeExtractor{}.Extract("/tmp/does_not_exist.flac", bad), "missing file rejected");
    });

    return Finish();
}
