#include "ResourceManager.hpp"

namespace flachead::resource
{
bool ImageLoader::Load(std::string_view)
{
    return true;
}

void ResourceManager::Initialize()
{
    m_ThemeManager.Load("default");
}

void ResourceManager::Shutdown()
{
}

flachead::graphics::TextureCache& ResourceManager::Textures()
{
    return m_TextureCache;
}

flachead::graphics::FontManager& ResourceManager::Fonts()
{
    return m_FontManager;
}

flachead::theme::ThemeManager& ResourceManager::Themes()
{
    return m_ThemeManager;
}

ImageLoader& ResourceManager::Images()
{
    return m_ImageLoader;
}
} // namespace flachead::resource
