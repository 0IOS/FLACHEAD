#include "Window.hpp"

#include <iostream>

Window::Window()
    : m_Window(nullptr)
{
}

Window::~Window()
{
    Destroy();
}

bool Window::Create()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL: "
                  << SDL_GetError() << '\n';
        return false;
    }

    m_Window = SDL_CreateWindow(
        "FLACHEAD",
        900,
        600,
        SDL_WINDOW_RESIZABLE
    );

    if (!m_Window)
    {
        std::cerr << "Failed to create window: "
                  << SDL_GetError() << '\n';

        SDL_Quit();
        return false;
    }

    return true;
}

void Window::Destroy()
{
    if (m_Window)
    {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }

    SDL_Quit();
}

bool Window::PollEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                return false;

            default:
                break;
        }
    }

    return true;
}

WindowSize Window::GetSize() const
{
    int width = 0;
    int height = 0;

    SDL_GetWindowSize(m_Window, &width, &height);

    return { width, height };
}

SDL_Window* Window::GetNativeWindow() const
{
    return m_Window;
}