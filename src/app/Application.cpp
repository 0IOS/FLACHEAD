#include "Application.hpp"
#include <SDL3/SDL.h>
#include <iostream>

Application::Application()
    : m_Running(false)
{
}

Application::~Application()
{
}

bool Application::Initialize()
{
    std::cout << "Initializing FLACHEAD...\n";

    if (!m_Window.Create())
        return false;

    m_Running = true;

    return true;
}
void Application::Run()
{
    while (m_Running)
    {
        m_Running = m_Window.PollEvents();

        m_Window.BeginFrame();

        // Drawing will go here

        m_Window.EndFrame();

        SDL_Delay(16);
    }
}
void Application::Shutdown()
{
    m_Window.Destroy();

    std::cout << "Shutting Down...\n";
}