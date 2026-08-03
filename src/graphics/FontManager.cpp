#include "FontManager.hpp"

namespace flachead::graphics
{
std::shared_ptr<Font> FontManager::Acquire(std::string_view path, float size)
{
    const int sizeKey = static_cast<int>(size);

    auto pathIt = m_Cache.find(path);
    if (pathIt != m_Cache.end())
    {
        auto sizeIt = pathIt->second.find(sizeKey);
        if (sizeIt != pathIt->second.end())
        {
            return sizeIt->second;
        }
    }

    auto font = std::make_shared<Font>(path, size);
    m_Cache[std::string{path}][sizeKey] = font;
    return font;
}

void FontManager::ReleaseAll()
{
    m_Cache.clear();
}
} // namespace flachead::graphics
