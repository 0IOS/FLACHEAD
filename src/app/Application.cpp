#include "Application.hpp"

#include "../core/Time.hpp"
#include <SDL3/SDL.h>

#include <iostream>

namespace
{
constexpr float kFpsSampleSeconds = 1.0f;
}

Application::Application()
    : m_Animator([this](float) {
          // The engine animator is available for future screen and widget transitions.
      })
{
}

Application::~Application()
{
    Shutdown();
}

bool Application::Initialize()
{
    flachead::core::Logger::Info("Starting FLACHEAD engine");

    if (!m_Window.Create())
    {
        return false;
    }

    if (!m_Renderer.Create(m_Window.GetNativeWindow()))
    {
        return false;
    }

    m_ThemeManager.Load("default");
    m_AppManager.Initialize();
    m_AudioService.Initialize();
    m_Canvas = new flachead::ui::Canvas(m_Renderer);
    m_Running = true;

    return true;
}

void Application::Run()
{
    float fpsTimer = 0.0f;

    while (m_Running)
    {
        Time::Update();
        const float deltaSeconds = Time::DeltaTime();
        fpsTimer += deltaSeconds;

        if (fpsTimer >= kFpsSampleSeconds)
        {
            flachead::core::Logger::Debug(std::to_string(Time::FPS()));
            fpsTimer = 0.0f;
        }

        m_Animator.Tick(deltaSeconds);
        m_AppManager.Tick(deltaSeconds);
        m_Running = m_Window.PollEvents();

        const flachead::system::WindowSize size = m_Window.GetSize();
        m_Renderer.BeginFrame();
        m_HomeScreen.Draw(*m_Canvas, size.width, size.height);
        m_Renderer.EndFrame();

        SDL_Delay(1);
    }
}

void Application::Shutdown()
{
    if (!m_Running)
    {
        return;
    }

    m_AudioService.Shutdown();
    m_AppManager.Shutdown();
    m_Renderer.Destroy();
    m_Window.Destroy();
    delete m_Canvas;
    m_Canvas = nullptr;
    m_Running = false;
}