#pragma once

#include "Font.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace flachead::graphics
{
class FontManager
{
public:
    std::shared_ptr<Font> Acquire(std::string_view path);

private:
    std::unordered_map<std::string, std::shared_ptr<Font>> m_Cache;
};
} // namespace flachead::graphics
