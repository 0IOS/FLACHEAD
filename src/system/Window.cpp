#include "Window.hpp"

#include <iostream>

Window::Window()
    : m_Window(nullptr),
      m_Renderer(nullptr)
{
}

Window::~Window()
{
    Destroy();
}

bool Window::Create()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "Failed to initialize SDL video: " << SDL_GetError() << '\n';
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
        std::cerr << "Failed to create window: " << SDL_GetError() << '\n';
        SDL_Quit();
        return false;
    }

    m_Renderer = SDL_CreateRenderer(m_Window, nullptr);

    if (!m_Renderer)
    {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << '\n';
        Destroy();
        return false;
    }

    return true;
}

void Window::Destroy()
{
    if (m_Renderer)
    {
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }

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
        if (event.type == SDL_EVENT_QUIT)
            return false;
    }

    return true;
}

void Window::BeginFrame()
{
    // Background
    SDL_SetRenderDrawColor(m_Renderer, 25, 25, 25, 255);
    SDL_RenderClear(m_Renderer);

    // White rectangle
    SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);

    SDL_FRect rect = {
        300.0f,
        200.0f,
        300.0f,
        200.0f
    };

    SDL_RenderFillRect(m_Renderer, &rect);
}

void Window::EndFrame()
{
    SDL_RenderPresent(m_Renderer);
}