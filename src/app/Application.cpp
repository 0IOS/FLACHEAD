// FLACHEAD - Application.cpp
#include "Application.hpp"

#include ".../core/Time.hpp"
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
        Time::Update();
        static float timer = 0.0f;
        timer += Time::DeltaTime();

        if (timer >= 1.0f)
        {
            std::cout << "FPS: " << Time::FPS() << '\n';
            timer = 0.0f;
        }
        m_Running = m_Window.PollEvents();

        WindowSize size = m_Window.GetSize();

        m_Renderer.BeginFrame();

        m_HomeScreen.Draw(
            *m_Canvas,
            size.width,
            size.height
        );

        m_Renderer.EndFrame();

        SDL_Delay(1);
    }
}

void Application::Shutdown()
{
    m_Renderer.Destroy();
    m_Window.Destroy();
    delete m_Canvas;
    m_Canvas=nullptr;
}