#ifdef FLACHEAD_HAVE_TAGLIB

#include "TagLibExtractor.hpp"

#include "../core/PathUtils.hpp"

#include <sys/stat.h>

#include <cstdlib>

#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

namespace flachead::metadata
{
namespace
{
int ParseNumber(const std::string& value)
{
    const std::size_t slash = value.find('/');
    const std::string head = slash != std::string::npos ? value.substr(0, slash) : value;
    return head.empty() ? 0 : std::atoi(head.c_str());
}
} // namespace

bool TagLibExtractor::Extract(std::string_view path, TrackMetadata& out)
{
    struct stat st{};
    if (stat(std::string{path}.c_str(), &st) != 0)
    {
        return false;
    }

    out = TrackMetadata{};
    out.fileSize = static_cast<std::int64_t>(st.st_size);
    out.fileMtime = static_cast<std::int64_t>(st.st_mtime);

    TagLib::FileRef ref(std::string{path}.c_str(), true, TagLib::AudioProperties::Accurate);
    if (ref.isNull())
    {
        return false;
    }

    if (TagLib::Tag* tag = ref.tag())
    {
        out.title = tag->title().to8Bit(true);
        out.artist = tag->artist().to8Bit(true);
        out.album = tag->album().to8Bit(true);
        out.genre = tag->genre().to8Bit(true);
        out.year = tag->year();
        out.trackNo = tag->track();
    }

    if (TagLib::AudioProperties* props = ref.audioProperties())
    {
        out.duration = props->length();
        out.bitrate = props->bitrate();
        out.sampleRate = props->sampleRate();
        out.channels = props->channels();
    }

    // Album artist and disc number live in extended tag maps, not Tag::*.
    if (ref.file())
    {
        const TagLib::PropertyMap properties = ref.file()->properties();
        if (properties.contains("ALBUMARTIST"))
        {
            const TagLib::StringList values = properties["ALBUMARTIST"];
            if (!values.isEmpty())
            {
                out.albumArtist = values.front().to8Bit(true);
            }
        }
        if (properties.contains("DISCNUMBER"))
        {
            const TagLib::StringList values = properties["DISCNUMBER"];
            if (!values.isEmpty())
            {
                out.discNo = ParseNumber(values.front().to8Bit(true));
            }
        }
    }

    if (out.codec.empty())
    {
        out.codec = flachead::core::path::Extension(path);
        if (!out.codec.empty())
        {
            out.codec.erase(out.codec.begin());
        }
    }

    return true;
}
} // namespace flachead::metadata

#endif // FLACHEAD_HAVE_TAGLIB
