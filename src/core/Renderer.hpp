#pragma once
#include "../math/Color.hpp"
#include "../math/Rect.hpp"
#include <SDL3/SDL.h>

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

private:
    SDL_Renderer* m_Renderer;
};