#pragma once

#include "Metadata.hpp"

#include <string_view>

namespace flachead::metadata
{
// Metadata backend built on TagLib, compiled only when FLACHEAD_HAVE_TAGLIB
// is defined (see CMakeLists.txt). Prefer this on targets where TagLib is
// installed: it is faster than spawning ffprobe per file.
class TagLibExtractor final : public MetadataExtractor
{
public:
    bool Extract(std::string_view path, TrackMetadata& out) override;
    std::string_view Name() const override { return "taglib"; }
};
} // namespace flachead::metadata
