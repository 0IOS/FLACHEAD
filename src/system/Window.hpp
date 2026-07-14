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

    SDL_Window* GetNativeWindow() const;

private:
    SDL_Window* m_Window;
};