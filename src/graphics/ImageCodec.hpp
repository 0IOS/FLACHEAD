#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace flachead::graphics
{
struct DecodedImage
{
    int width{0};
    int height{0};
    std::vector<uint8_t> rgba;
};

// Decodes PNG, JPEG and BMP files into RGBA8 pixels. Returns false when the
// file cannot be decoded or has no usable dimensions. BMP support comes from
// SDL; PNG and JPEG are decoded by libpng / libjpeg.
bool DecodeImageFile(const std::string& path, DecodedImage& out);
} // namespace flachead::graphics
