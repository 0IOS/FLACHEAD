#pragma once

#include "Font.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace flachead::graphics
{
class FontManager
{
public:
    std::shared_ptr<Font> Acquire(std::string_view path, float size);
    void ReleaseAll();

private:
    std::map<std::string, std::map<int, std::shared_ptr<Font>>, std::less<>> m_Cache;
};
} // namespace flachead::graphics
