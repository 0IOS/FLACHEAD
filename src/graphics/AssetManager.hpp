#pragma once

#include "../math/Color.hpp"

#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace flachead::core
{
class Renderer;
}

namespace flachead::graphics
{
// Procedurally-generated texture cache. Small, reusable assets (solid panels,
// gradients, cover placeholders) are produced once against the renderer and
// reused by key. All textures are discarded when the renderer is recreated.
class AssetManager
{
public:
    explicit AssetManager(flachead::core::Renderer& renderer);

    std::shared_ptr<SDL_Texture> Solid(int width, int height, const Color& color);
    std::shared_ptr<SDL_Texture> Gradient(int width, int height, const Color& top, const Color& bottom);
    std::shared_ptr<SDL_Texture> RoundedPanel(int width, int height, float radius, const Color& fill,
                                              const Color& border, float borderWidth);

    void Clear();
    size_t Count() const { return m_Assets.size(); }

private:
    std::string Key(std::string_view kind, int width, int height, const Color& a, const Color& b) const;

    flachead::core::Renderer& m_Renderer;
    std::unordered_map<std::string, std::shared_ptr<SDL_Texture>> m_Assets;
};
} // namespace flachead::graphics
