#include "Application.hpp"

#include "../core/Time.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

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

    std::cout << "Starting FLACHEAD...\n";

    if (!TTF_Init())
    {
        std::cerr << "Failed to initialize SDL_ttf\n";
        return false;
    }

    if (!m_Window.Create())
    {
        return false;
    }

    if (!m_Renderer.Create(m_Window.GetNativeWindow()))
    {
        return false;
    }

    m_ThemeManager.Load("default");
    m_Resources.Initialize();
    m_AppManager.Initialize();
    m_AudioService.Initialize();
    m_Canvas = new flachead::ui::Canvas(m_Renderer, m_FontManager, m_ThemeManager);

    m_LauncherScreen = std::make_unique<HomeScreen>();
    m_MusicScreen = std::make_unique<flachead::apps::MusicScreen>();
    m_GalleryScreen = std::make_unique<flachead::apps::GalleryScreen>();
    m_VideoScreen = std::make_unique<flachead::apps::VideoScreen>();
    m_CalculatorScreen = std::make_unique<flachead::apps::CalculatorScreen>();
    m_CalendarScreen = std::make_unique<flachead::apps::CalendarScreen>();
    m_NotesScreen = std::make_unique<flachead::apps::NotesScreen>();
    m_SettingsScreen = std::make_unique<flachead::apps::SettingsScreen>();
    m_FileBrowserScreen = std::make_unique<flachead::apps::FileBrowserScreen>();
    m_PowerScreen = std::make_unique<flachead::apps::PowerScreen>();

    m_LauncherScreen->SetLaunchHandler([this](std::string_view app) {
        if (app == "Music")
        {
            m_ScreenManager.Push("music");
        }
        else if (app == "Gallery")
        {
            m_ScreenManager.Push("gallery");
        }
        else if (app == "Video")
        {
            m_ScreenManager.Push("video");
        }
        else if (app == "Calculator")
        {
            m_ScreenManager.Push("calculator");
        }
        else if (app == "Calendar")
        {
            m_ScreenManager.Push("calendar");
        }
        else if (app == "Notes")
        {
            m_ScreenManager.Push("notes");
        }
        else if (app == "Settings")
        {
            m_ScreenManager.Push("settings");
        }
        else if (app == "File Browser")
        {
            m_ScreenManager.Push("filebrowser");
        }
        else if (app == "Power")
        {
            m_ScreenManager.Push("power");
        }
    });
    m_LauncherScreen->SetBackHandler([this] {
        m_Running = false;
    });

    m_MusicScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });
    m_GalleryScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });
    m_VideoScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });
    m_CalculatorScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });
    m_CalendarScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });
    m_NotesScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });
    m_SettingsScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });
    m_FileBrowserScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });
    m_PowerScreen->SetBackHandler([this] {
        m_ScreenManager.Pop();
    });

    m_ScreenManager.Register("launcher", std::move(m_LauncherScreen));
    m_ScreenManager.Register("music", std::move(m_MusicScreen));
    m_ScreenManager.Register("gallery", std::move(m_GalleryScreen));
    m_ScreenManager.Register("video", std::move(m_VideoScreen));
    m_ScreenManager.Register("calculator", std::move(m_CalculatorScreen));
    m_ScreenManager.Register("calendar", std::move(m_CalendarScreen));
    m_ScreenManager.Register("notes", std::move(m_NotesScreen));
    m_ScreenManager.Register("settings", std::move(m_SettingsScreen));
    m_ScreenManager.Register("filebrowser", std::move(m_FileBrowserScreen));
    m_ScreenManager.Register("power", std::move(m_PowerScreen));

    m_ScreenManager.Push("launcher");
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
        m_ScreenManager.Update(deltaSeconds);
        m_Running = m_Window.PollEvents([this](const SDL_Event& event) {
            if (auto* screen = m_ScreenManager.Current())
            {
                return screen->HandleEvent(event);
            }
            return false;
        });

        const flachead::system::WindowSize size = m_Window.GetSize();
        m_Renderer.BeginFrame();
        m_ScreenManager.Render(*m_Canvas, size.width, size.height);
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
    m_Resources.Shutdown();
    m_Renderer.Destroy();
    m_Window.Destroy();
    delete m_Canvas;
    m_Canvas = nullptr;
    m_Running = false;
    TTF_Quit();
}