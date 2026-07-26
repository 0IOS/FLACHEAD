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
    void FillRoundedRect(const Rect& rect, float radius);
    void DrawRoundedRect(const Rect& rect, float radius);
    void DrawLine(float x1, float y1, float x2, float y2);
    void FillCircle(float cx, float cy, float radius);
    void DrawCircle(float cx, float cy, float radius);
    void DrawText(const Rect& rect, std::string_view text, const flachead::graphics::Font& font, const Color& color);

    SDL_Renderer* GetSDLRenderer() const { return m_Renderer; }

private:
    void RenderCirclePoints(int cx, int cy, int x, int y, bool fill);

    SDL_Renderer* m_Renderer{nullptr};
};
} // namespace flachead::core