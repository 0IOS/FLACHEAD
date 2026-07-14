// FLACHEAD - Application.cpp
#include "Application.hpp"

#include <SDL3/SDL.h>
#include <iostream>

Application::Application()
    : m_Running(false),
    m_Canvas(nullptr)
{
}

Application::~Application()
{
    Shutdown();
}

bool Application::Initialize()
{
    std::cout << "Starting FLACHEAD...\n";

    if (!m_Window.Create())
        return false;

    if (!m_Renderer.Create(m_Window.GetNativeWindow()))
        return false;

    m_Canvas = new Canvas(m_Renderer);
    
    m_Running = true;

    return true;
}

void Application::Run()
{
    while (m_Running)
    {
        m_Running = m_Window.PollEvents();

        m_Renderer.BeginFrame();

        // Demo rectangle
       Rect rect(250,150,400,300);

        m_Canvas->FillRect(rect, Color::White);

        m_Renderer.EndFrame();

        SDL_Delay(16);
    }
}

void Application::Shutdown()
{
    m_Renderer.Destroy();
    m_Window.Destroy();
    delete m_Canvas;
    m_Canvas=nullptr;
}