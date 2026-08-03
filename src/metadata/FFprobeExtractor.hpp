#pragma once

#include "Metadata.hpp"

#include <string_view>

namespace flachead::metadata
{
// Metadata backend that shells out to the `ffprobe` binary and parses its
// JSON output. ffprobe ships with ffmpeg and is installed on the target
// image; this backend needs no development headers.
class FFprobeExtractor final : public MetadataExtractor
{
public:
    bool Extract(std::string_view path, TrackMetadata& out) override;
    std::string_view Name() const override { return "ffprobe"; }

    // True when `ffprobe` is reachable on PATH.
    static bool Available();
};
} // namespace flachead::metadata
