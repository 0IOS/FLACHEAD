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

    WindowSize GetSize() const;
    SDL_Window* GetNativeWindow() const;

    bool Create();
    void Destroy();

    bool PollEvents();

    SDL_Window* GetNativeWindow() const;

private:
    SDL_Window* m_Window;
};

WindowSize Window::GetSize() const
{
    int width;
    int height;

    SDL_GetWindowSize(m_Window, &width, &height);

    return { width, height };
}

SDL_Window* Window::GetNativeWindow() const
{
    return m_Window;
}