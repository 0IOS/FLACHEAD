#pragma once

#include "../graphics/Font.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"
#include <SDL3/SDL.h>
#include <string_view>

namespace flachead::core
{
class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool Create(SDL_Window* window);
    void Destroy();

    void BeginFrame();
    void EndFrame();

    void SetColor(const Color& color);

    void DrawRect(const Rect& rect);
    void FillRect(const Rect& rect);
    void DrawText(const Rect& rect, std::string_view text, const flachead::graphics::Font& font, const Color& color);

private:
    SDL_Renderer* m_Renderer{nullptr};
};
} // namespace flachead::core