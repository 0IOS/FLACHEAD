#pragma once

#include <SDL3/SDL.h>

struct WindowSize
{
    int width;
    int height;
};

class Window
{
public:
    Window();
    ~Window();

    bool Create();
    void Destroy();

    bool PollEvents();

    SDL_Window* GetNativeWindow() const;
    WindowSize GetSize() const;

private:
    SDL_Window* m_Window;
};