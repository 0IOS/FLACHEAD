#pragma once

#include "../graphics/FontManager.hpp"
#include "../graphics/TextureCache.hpp"
#include "../ui/theme/ThemeManager.hpp"

#include <memory>
#include <string>
#include <string_view>

namespace flachead::resource
{
class ImageLoader
{
public:
    bool Load(std::string_view path);
};

class ResourceManager
{
public:
    ResourceManager() = default;

    void Initialize();
    void Shutdown();

    flachead::graphics::TextureCache& Textures();
    flachead::graphics::FontManager& Fonts();
    flachead::theme::ThemeManager& Themes();
    ImageLoader& Images();

private:
    flachead::graphics::TextureCache m_TextureCache;
    flachead::graphics::FontManager m_FontManager;
    flachead::theme::ThemeManager m_ThemeManager;
    ImageLoader m_ImageLoader;
};
} // namespace flachead::resource
