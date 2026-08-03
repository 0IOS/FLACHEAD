#include "Application.hpp"

#include "../apps/AppScreens.hpp"
#include "../core/Time.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cmath>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <iostream>
namespace
{
constexpr float kFpsSampleSeconds = 1.0f;
constexpr float kIdleWaitMs       = 50;
constexpr int   kGpioPollMs       = 20;
constexpr int   kFrameTierFloor   = 30;
constexpr float kFrameEmaAlpha    = 0.1f;
constexpr float kTierDegradeRatio = 1.10f;
constexpr float kTierPromoteRatio = 0.75f;
constexpr int   kTierStability    = 30;
} // namespace

long PeakRssKb()
{
#if defined(__linux__)
    FILE* status = std::fopen("/proc/self/status", "r");
    if (!status)
    {
        return -1;
    }

    long peakKb = -1;
    char line[256];
    while (std::fgets(line, sizeof(line), status))
    {
        if (std::sscanf(line, "VmHWM: %ld kB", &peakKb) == 1)
        {
            break;
        }
    }
    std::fclose(status);
    return peakKb;
#else
    return -1;
#endif
}

Application::Application(float benchmarkSeconds, std::string_view inputBackend)
    : m_Animator([this](float) {
          // The engine animator is available for future screen and widget transitions.
      }),
      m_BenchmarkSeconds(benchmarkSeconds)
{
    if (inputBackend == "gpio")
    {
        auto gpio = std::make_unique<flachead::input::GpioInputBackend>();
        if (gpio->Initialize())
        {
            m_OwnedInputBackend = std::move(gpio);
            m_InputBackend = m_OwnedInputBackend.get();
        }
        else
        {
            flachead::core::Logger::Warning("GPIO input unavailable, falling back to SDL input");
        }
    }

    if (!m_InputBackend)
    {
        m_OwnedInputBackend = std::make_unique<flachead::input::SdlInputBackend>();
        m_OwnedInputBackend->Initialize();
        m_InputBackend = m_OwnedInputBackend.get();
    }
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

    RegisterScreens();

    m_ScreenManager.Push("launcher");
    m_Running = true;

    return true;
}

void Application::RegisterScreens()
{
    m_ScreenManager.RegisterFactory("launcher", [this] {
        auto screen = std::make_unique<HomeScreen>();
        screen->SetLaunchHandler([this](std::string_view app) {
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
        screen->SetBackHandler([this] {
            m_Running = false;
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("music", [this] {
        auto screen = std::make_unique<flachead::apps::MusicScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("gallery", [this] {
        auto screen = std::make_unique<flachead::apps::GalleryScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("video", [this] {
        auto screen = std::make_unique<flachead::apps::VideoScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("calculator", [this] {
        auto screen = std::make_unique<flachead::apps::CalculatorScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("calendar", [this] {
        auto screen = std::make_unique<flachead::apps::CalendarScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("notes", [this] {
        auto screen = std::make_unique<flachead::apps::NotesScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("settings", [this] {
        auto screen = std::make_unique<flachead::apps::SettingsScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("filebrowser", [this] {
        auto screen = std::make_unique<flachead::apps::FileBrowserScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });

    m_ScreenManager.RegisterFactory("power", [this] {
        auto screen = std::make_unique<flachead::apps::PowerScreen>();
        screen->SetBackHandler([this] {
            m_ScreenManager.Pop();
        });
        return screen;
    });
}

void Application::RenderFrame(int width, int height)
{
    m_Renderer.BeginFrame();
    m_ScreenManager.Render(*m_Canvas, width, height);
    m_Renderer.EndFrame();
}

void Application::UpdateFrameTier(float renderMs)
{
    if (renderMs > 0.0f)
    {
        m_FrameTimeEma = m_FrameTimeEma * (1.0f - kFrameEmaAlpha) + renderMs * kFrameEmaAlpha;
    }

    const float budget = 1000.0f / static_cast<float>(m_FrameTier);
    const bool degrade = m_FrameTier > kFrameTierFloor && m_FrameTimeEma > budget * kTierDegradeRatio;

    const int nextTier = (m_FrameTier == kFrameTierFloor) ? 45 : 60;
    const bool promote = m_FrameTier < 60 && m_FrameTimeEma < (1000.0f / static_cast<float>(nextTier)) * kTierPromoteRatio;

    if (degrade || promote)
    {
        if (++m_TierStabilityFrames >= kTierStability)
        {
            m_TierStabilityFrames = 0;
            m_FrameTier = degrade ? ((m_FrameTier == 60) ? 45 : kFrameTierFloor) : nextTier;
            flachead::core::Logger::Info("Adaptive frame tier: " + std::to_string(m_FrameTier) + " FPS");
        }
    }
    else
    {
        m_TierStabilityFrames = 0;
    }
}

void Application::Run()
{
    const Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
    Uint64 lastFrameCounter = SDL_GetPerformanceCounter();
    std::time_t lastMinute = std::time(nullptr) / 60;
    int lastWidth = -1;
    int lastHeight = -1;

    while (m_Running)
    {
        Time::Update();
        const float deltaSeconds = Time::DeltaTime();

        m_FpsAccumulator += deltaSeconds;
        if (m_FpsAccumulator >= kFpsSampleSeconds)
        {
            if (m_BenchmarkSeconds > 0.0f)
            {
                if (m_FramesThisSecond < m_MinFpsSecond)
                {
                    m_MinFpsSecond = m_FramesThisSecond;
                }
                if (m_FramesThisSecond > m_MaxFpsSecond)
                {
                    m_MaxFpsSecond = m_FramesThisSecond;
                }
            }
            else
            {
                flachead::core::Logger::Debug(std::to_string(Time::FPS()));
            }
            m_FpsAccumulator = 0.0f;
            m_FramesThisSecond = 0;
        }

        m_Animator.Tick(deltaSeconds);
        m_AppManager.Tick(deltaSeconds);
        m_ScreenManager.Update(deltaSeconds);

        bool handled = false;
        m_InputBackend->Poll([this, &handled](const SDL_Event& event) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_TERMINATING)
            {
                m_Running = false;
                handled = true;
                return;
            }
            if (auto* screen = m_ScreenManager.Current())
            {
                handled = screen->HandleEvent(event) || handled;
            }
        });

        const flachead::system::WindowSize size = m_Window.GetSize();
        const bool resized = size.width != lastWidth || size.height != lastHeight;
        lastWidth = size.width;
        lastHeight = size.height;

        const std::time_t minute = std::time(nullptr) / 60;
        const bool minuteChanged = minute != lastMinute;
        lastMinute = minute;

        if (handled || resized || minuteChanged || m_ScreenManager.NeedsRender() || m_BenchmarkSeconds > 0.0f)
        {
            const Uint64 renderStart = SDL_GetPerformanceCounter();
            RenderFrame(size.width, size.height);
            const Uint64 renderEnd = SDL_GetPerformanceCounter();
            const float renderMs = static_cast<float>(renderEnd - renderStart) * 1000.0f
                                   / static_cast<float>(performanceFrequency);

            ++m_FramesThisSecond;
            ++m_FrameCount;
            const float frameMs = deltaSeconds * 1000.0f;
            m_TotalFrameMs += frameMs;
            if (frameMs > m_WorstFrameMs)
            {
                m_WorstFrameMs = frameMs;
            }

            if (m_BenchmarkSeconds <= 0.0f)
            {
                UpdateFrameTier(renderMs);
            }

            const float targetMs = 1000.0f / static_cast<float>(m_FrameTier);
            const Uint64 now = SDL_GetPerformanceCounter();
            const Uint64 elapsed = now - lastFrameCounter;
            const Uint64 target = static_cast<Uint64>(targetMs * 0.001f * static_cast<float>(performanceFrequency));
            if (elapsed < target)
            {
                SDL_Delay(static_cast<int>((target - elapsed) * 1000 / performanceFrequency));
            }
            lastFrameCounter = SDL_GetPerformanceCounter();
        }
        else
        {
            if (std::strcmp(m_InputBackend->Name(), "gpio") == 0)
            {
                SDL_Delay(kGpioPollMs);
            }
            else
            {
                m_Window.WaitForEvent(kIdleWaitMs);
            }
        }

        if (m_BenchmarkSeconds > 0.0f && m_FrameCount >= static_cast<std::uint32_t>(m_BenchmarkSeconds * 60.0f))
        {
            break;
        }
    }

    if (m_BenchmarkSeconds > 0.0f && m_FrameCount > 0)
    {
        std::printf("=== FLACHEAD BENCHMARK ===\n");
        std::printf("Duration        : %.1f s\n", m_BenchmarkSeconds);
        std::printf("Frames rendered : %u\n", m_FrameCount);
        std::printf("Average FPS     : %.1f\n", static_cast<float>(m_FrameCount) / m_BenchmarkSeconds);
        std::printf("Min FPS (sec)   : %u\n", m_MinFpsSecond == 0xFFFFFFFFu ? 0 : m_MinFpsSecond);
        std::printf("Max FPS (sec)   : %u\n", m_MaxFpsSecond);
        std::printf("Avg frame time  : %.3f ms\n", m_TotalFrameMs / static_cast<float>(m_FrameCount));
        std::printf("Worst frame     : %.3f ms\n", m_WorstFrameMs);

        const flachead::core::Renderer::Stats rendererStats = m_Renderer.GetStats();
        std::printf("Textures        : %d (%.1f KB)\n", rendererStats.textureCount,
                    static_cast<double>(rendererStats.textureBytes) / 1024.0);

        const long peakRssKb = PeakRssKb();
        if (peakRssKb > 0)
        {
            std::printf("Peak RSS        : %ld KB\n", peakRssKb);
        }
        std::printf("==========================\n");
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
    m_FontManager.ReleaseAll();
    m_Window.Destroy();
    delete m_Canvas;
    m_Canvas = nullptr;
    m_Running = false;
    TTF_Quit();
}
