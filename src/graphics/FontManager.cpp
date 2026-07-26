#include "FontManager.hpp"

namespace flachead::graphics
{
std::shared_ptr<Font> FontManager::Acquire(std::string_view path)
{
    auto it = m_Cache.find(std::string{path});
    if (it != m_Cache.end())
    {
        return it->second;
    }

    auto font = std::make_shared<Font>(path);
    m_Cache.emplace(std::string{path}, font);
    return font;
}
} // namespace flachead::graphics
