// FLACHEAD - Application.cpp
#include "Application.hpp"

#include <SDL3/SDL.h>
#include <iostream>

Application::Application()
    : m_Running(false)
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
        m_Renderer.SetColor(255, 255, 255);
        m_Renderer.FillRect(250.0f, 150.0f, 400.0f, 300.0f);

        m_Renderer.EndFrame();

        SDL_Delay(16);
    }
}

void Application::Shutdown()
{
    m_Renderer.Destroy();
    m_Window.Destroy();
}