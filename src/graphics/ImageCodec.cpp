#include "ImageCodec.hpp"

#include <png.h>
#include <jpeglib.h>
#include <SDL3/SDL.h>

#include <csetjmp>
#include <cstdio>
#include <cstring>

namespace flachead::graphics
{
namespace
{
bool HasMagic(const std::string& path, const unsigned char* magic, std::size_t size)
{
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (!file)
    {
        return false;
    }
    unsigned char buffer[16] = {};
    const std::size_t read = std::fread(buffer, 1, size, file);
    std::fclose(file);
    return read == size && std::memcmp(buffer, magic, size) == 0;
}

bool IsPng(const std::string& path)
{
    static const unsigned char magic[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    return HasMagic(path, magic, sizeof(magic));
}

bool IsJpeg(const std::string& path)
{
    static const unsigned char magic[3] = {0xFF, 0xD8, 0xFF};
    return HasMagic(path, magic, sizeof(magic));
}

struct PngReader
{
    const unsigned char* data;
    std::size_t size;
    std::size_t offset;
};

void PngReadFn(png_structp png, png_bytep out, png_size_t count)
{
    auto* reader = static_cast<PngReader*>(png_get_io_ptr(png));
    const std::size_t available = reader->size - reader->offset;
    const std::size_t copy = available < count ? available : count;
    std::memcpy(out, reader->data + reader->offset, copy);
    reader->offset += copy;
}

bool DecodePng(const std::string& path, DecodedImage& out)
{
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (!file)
    {
        return false;
    }
    std::fseek(file, 0, SEEK_END);
    const long fileSize = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    if (fileSize <= 0)
    {
        std::fclose(file);
        return false;
    }

    std::vector<unsigned char> data(static_cast<std::size_t>(fileSize));
    const std::size_t read = std::fread(data.data(), 1, data.size(), file);
    std::fclose(file);
    if (read != data.size())
    {
        return false;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info = png ? png_create_info_struct(png) : nullptr;
    if (!png || !info)
    {
        if (png)
        {
            png_destroy_read_struct(&png, nullptr, nullptr);
        }
        return false;
    }

    bool ok = false;
    if (setjmp(png_jmpbuf(png)) == 0)
    {
        PngReader reader{data.data(), data.size(), 0};
        png_set_read_fn(png, &reader, PngReadFn);
        png_read_info(png, info);

        png_uint_32 width = 0;
        png_uint_32 height = 0;
        int bitDepth = 0;
        int colorType = 0;
        png_get_IHDR(png, info, &width, &height, &bitDepth, &colorType, nullptr, nullptr, nullptr);

        if (colorType == PNG_COLOR_TYPE_PALETTE)
        {
            png_set_palette_to_rgb(png);
        }
        if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
        {
            png_set_expand_gray_1_2_4_to_8(png);
        }
        if (png_get_valid(png, info, PNG_INFO_tRNS))
        {
            png_set_tRNS_to_alpha(png);
        }
        if (bitDepth == 16)
        {
            png_set_strip_16(png);
        }
        if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        {
            png_set_gray_to_rgb(png);
        }
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

        png_read_update_info(png, info);

        out.width = static_cast<int>(width);
        out.height = static_cast<int>(height);
        out.rgba.resize(static_cast<std::size_t>(width) * height * 4);

        std::vector<png_bytep> rows(static_cast<std::size_t>(height));
        for (png_uint_32 y = 0; y < height; ++y)
        {
            rows[y] = out.rgba.data() + static_cast<std::size_t>(y) * width * 4;
        }
        png_read_image(png, rows.data());
        png_read_end(png, nullptr);
        ok = true;
    }

    png_destroy_read_struct(&png, &info, nullptr);
    return ok;
}

bool DecodeJpeg(const std::string& path, DecodedImage& out)
{
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (!file)
    {
        return false;
    }
    std::fseek(file, 0, SEEK_END);
    const long fileSize = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    if (fileSize <= 0)
    {
        std::fclose(file);
        return false;
    }

    std::vector<unsigned char> data(static_cast<std::size_t>(fileSize));
    const std::size_t read = std::fread(data.data(), 1, data.size(), file);
    std::fclose(file);
    if (read != data.size())
    {
        return false;
    }

    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data.data(), data.size());
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
    {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    jpeg_start_decompress(&cinfo);
    out.width = static_cast<int>(cinfo.output_width);
    out.height = static_cast<int>(cinfo.output_height);
    const int components = cinfo.output_components;
    out.rgba.resize(static_cast<std::size_t>(out.width) * out.height * 4);

    std::vector<unsigned char> row(static_cast<std::size_t>(out.width) * components);
    for (int y = 0; y < out.height; ++y)
    {
        unsigned char* rowPointers[1] = {row.data()};
        jpeg_read_scanlines(&cinfo, rowPointers, 1);
        unsigned char* dest = out.rgba.data() + static_cast<std::size_t>(y) * out.width * 4;
        for (int x = 0; x < out.width; ++x)
        {
            const unsigned char* src = row.data() + static_cast<std::size_t>(x) * components;
            dest[x * 4] = src[0];
            dest[x * 4 + 1] = components >= 3 ? src[1] : src[0];
            dest[x * 4 + 2] = components >= 3 ? src[2] : src[0];
            dest[x * 4 + 3] = 255;
        }
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return true;
}

bool DecodeBmp(const std::string& path, DecodedImage& out)
{
    SDL_Surface* surface = SDL_LoadBMP(path.c_str());
    if (!surface)
    {
        return false;
    }
    SDL_Surface* rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA8888);
    SDL_DestroySurface(surface);
    if (!rgba)
    {
        return false;
    }
    out.width = rgba->w;
    out.height = rgba->h;
    out.rgba.resize(static_cast<std::size_t>(out.width) * out.height * 4);
    std::memcpy(out.rgba.data(), rgba->pixels, out.rgba.size());
    SDL_DestroySurface(rgba);
    return true;
}
} // namespace

bool DecodeImageFile(const std::string& path, DecodedImage& out)
{
    if (IsPng(path))
    {
        return DecodePng(path, out);
    }
    if (IsJpeg(path))
    {
        return DecodeJpeg(path, out);
    }
    return DecodeBmp(path, out);
}
} // namespace flachead::graphics
