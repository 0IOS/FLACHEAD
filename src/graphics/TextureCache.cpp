#include "TextureCache.hpp"

namespace flachead::graphics
{
std::shared_ptr<Texture> TextureCache::Acquire(std::string_view path)
{
    auto it = m_Cache.find(std::string{path});
    if (it != m_Cache.end())
    {
        return it->second;
    }

    auto texture = std::make_shared<Texture>(path);
    m_Cache.emplace(std::string{path}, texture);
    return texture;
}
} // namespace flachead::graphics
