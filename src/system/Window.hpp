#pragma once

#include <SDL3/SDL.h>

class Window
{
public:
    Window();
    ~Window();

    bool Create();
    void Destroy();

    bool PollEvents();

    void BeginFrame();
    void EndFrame();

private:
    SDL_Window* m_Window;
    SDL_Renderer* m_Renderer;   // <-- Add this
};