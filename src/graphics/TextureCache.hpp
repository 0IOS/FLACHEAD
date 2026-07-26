#pragma once

#include "Texture.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace flachead::graphics
{
class TextureCache
{
public:
    std::shared_ptr<Texture> Acquire(std::string_view path);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_Cache;
};
} // namespace flachead::graphics
