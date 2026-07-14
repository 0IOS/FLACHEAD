#pragma once

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

    void SetColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

    void DrawRect(float x, float y, float w, float h);
    void FillRect(float x, float y, float w, float h);

private:
    SDL_Renderer* m_Renderer;
};