#include "Application.hpp"

#include "../core/Time.hpp"
#include "../dap/LibraryScreens.hpp"
#include "../dap/PlaybackScreens.hpp"
#include "../dap/PlaylistScreens.hpp"
#include "../dap/SettingsScreen.hpp"
#include "../playback/QueueManager.hpp"
#include "../screens/HomeScreen.hpp"
#include "../shell/AmbientHomeScreen.hpp"
#include "../shell/LauncherScreen.hpp"
#include "../shell/ShellScreen.hpp"
#include "../shell/SettingsScreen.hpp"
#include "../shell/SystemScreen.hpp"
#include "../shell/TaskOverviewScreen.hpp"
#include "../shell/UniversalSearchScreen.hpp"
#include "../ui/Label.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <sys/stat.h>
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
      m_AudioService(m_EventBus),
      m_LibraryService(m_Database, m_EventBus),
      m_Playback(m_AudioService, m_EventBus),
      m_Playlists(m_Database, m_EventBus),
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

    SetupServices();
    SetupShellInput();
    RegisterScreens();

    m_ScreenManager.Push("home");
    m_Running = true;

    return true;
}

void Application::SetupShellInput()
{
    const flachead::system::WindowSize size = m_Window.GetSize();
    m_InputManager.Initialize();
    m_InputManager.SetWindowSize(size.width, size.height);
    m_InputManager.SetInputEventCallback([this](const flachead::input::InputEvent& event) {
        if (auto* screen = m_ScreenManager.Current())
        {
            screen->OnInputEvent(event);
        }
    });
    m_InputManager.SetCommandCallback([this](flachead::commands::Command command) {
        OnSystemCommand(command);
    });

    m_ShellServices.app = m_AppContext;
    m_ShellServices.renderer = &m_Renderer;
    m_ShellServices.themes = &m_ThemeManager;
    m_ShellServices.overlays = &m_Overlays;
    m_ShellServices.wallpaper = &m_Wallpaper;
    m_ShellServices.animations = &m_Animations;
    m_ShellServices.focus = &m_Focus;
    m_ShellServices.screens = &m_ScreenManager;
    m_ShellServices.backgroundJobs = &m_BackgroundJobs;
    m_ShellServices.notifications = &m_Notifications;
    m_ShellServices.memory = &m_Memory;
    m_ShellServices.quit = [this] { m_Running = false; };

    // Bridge notifications into the overlay layer as dismissible toasts.
    m_Notifications.SetToastHook([this](const flachead::services::Notification& notification) {
        auto toast = std::make_shared<flachead::ui::Label>();
        std::string text = notification.title;
        if (!notification.body.empty())
        {
            text += ": " + notification.body;
        }
        toast->SetText(text);
        m_Overlays.PushToast(notification.id, std::move(toast));
    });

    m_CommandCenter.Register(
        [this](flachead::commands::Command command) { return HandleSystemCommand(command); }, 0);
}

void Application::OnSystemCommand(flachead::commands::Command command)
{
    if (auto* screen = m_ScreenManager.Current())
    {
        if (screen->OnCommand(command))
        {
            return;
        }
    }
    m_CommandCenter.Dispatch(command);
}

bool Application::HandleSystemCommand(flachead::commands::Command command)
{
    switch (command)
    {
        case flachead::commands::Command::Back:
            // Legacy DAP screens already pop themselves on the raw Escape key;
            // applying Back here too would pop two screens. The shell handles
            // Back through its own OnShellCommand when it wants it.
            if (IsShellTop() && m_ScreenManager.Depth() > 1)
            {
                m_ScreenManager.Pop();
            }
            return true;
        case flachead::commands::Command::Launcher:
            if (m_ScreenManager.Top() != "launcher")
            {
                m_ScreenManager.Push("launcher");
            }
            return true;
        case flachead::commands::Command::Home:
            m_ScreenManager.PopTo("home");
            return true;
        case flachead::commands::Command::TaskOverview:
            if (m_ScreenManager.Top() != "taskoverview")
            {
                m_ScreenManager.Push("taskoverview");
            }
            return true;
        case flachead::commands::Command::OpenSettings:
            if (m_ScreenManager.Top() != "settings")
            {
                m_ScreenManager.Push("settings");
            }
            return true;
        case flachead::commands::Command::OpenSearch:
            if (m_ScreenManager.Top() != "universal_search")
            {
                m_ScreenManager.Push("universal_search");
            }
            return true;
        case flachead::commands::Command::OpenQueue:
            if (m_ScreenManager.Top() != "queue")
            {
                m_ScreenManager.Push("queue");
            }
            return true;
        case flachead::commands::Command::Shutdown:
            m_Running = false;
            return true;
        default:
            break;
    }

    // Playback commands are only applied globally when the shell owns the
    // screen; legacy DAP screens still handle media keys from their raw SDL
    // event path, and applying them again here would double-toggle.
    if (IsShellTop())
    {
        switch (command)
        {
            case flachead::commands::Command::PlayPause:
                m_Playback.Toggle();
                return true;
            case flachead::commands::Command::Next:
                m_Playback.Next();
                return true;
            case flachead::commands::Command::Previous:
                m_Playback.Previous();
                return true;
            case flachead::commands::Command::ToggleShuffle:
                m_Playback.ToggleShuffle();
                return true;
            case flachead::commands::Command::ToggleRepeat:
                m_Playback.ToggleRepeat();
                return true;
            case flachead::commands::Command::VolumeUp:
                m_Playback.SetVolume(std::min(1.0f, m_Playback.Volume() + 0.05f));
                return true;
            case flachead::commands::Command::VolumeDown:
                m_Playback.SetVolume(std::max(0.0f, m_Playback.Volume() - 0.05f));
                return true;
            default:
                break;
        }
    }
    return false;
}

bool Application::IsShellTop() const
{
    return dynamic_cast<flachead::shell::ShellScreen*>(m_ScreenManager.Current()) != nullptr;
}

void Application::SetupServices()
{
    const char* home = std::getenv("HOME");
    const std::string baseDir = home ? std::string(home) + "/.flachead" : "/tmp/flachead";

    if (::mkdir(baseDir.c_str(), 0755) != 0 && errno != EEXIST)
    {
        flachead::core::Logger::Warning("Could not create config dir " + baseDir);
    }

    const std::string dbPath = baseDir + "/flachead.db";
    if (!m_Database.Open(dbPath))
    {
        flachead::core::Logger::Warning("Failed to open database " + dbPath);
    }
    else
    {
        m_Database.Migrate();
    }

    m_SettingsManager.Initialize(m_Database, m_EventBus);
    m_Playback.Initialize();

    // Persist volume through settings and count plays through the library.
    m_Playback.SetVolumeStore([this](float volume) {
        m_SettingsManager.SetFloat("audio.volume", volume);
    });
    m_Playback.SetPlayHook([this](const flachead::models::SongModel& song) {
        m_LibraryService.MarkPlayed(song);
    });

    // Restore persisted player state.
    m_Playback.SetVolume(m_SettingsManager.GetFloat("audio.volume", 0.8f));
    m_Playback.SetRepeat(static_cast<flachead::playback::RepeatMode>(
        m_SettingsManager.GetInt("playback.repeat", 0)));
    if (m_SettingsManager.GetBool("playback.shuffle", false))
    {
        m_Playback.ToggleShuffle();
    }

    // Scan roots from settings, defaulting to $HOME/Music.
    const std::string configuredRoot = m_SettingsManager.Get("library.scan_roots");
    if (!configuredRoot.empty())
    {
        m_ScanRoots.push_back(configuredRoot);
    }
    else
    {
        const std::string defaultRoot = home ? std::string(home) + "/Music" : "";
        if (!defaultRoot.empty())
        {
            struct stat st;
            if (::stat(defaultRoot.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
            {
                m_ScanRoots.push_back(defaultRoot);
            }
        }
    }

    m_AppContext.eventBus = &m_EventBus;
    m_AppContext.library = &m_LibraryService;
    m_AppContext.playback = &m_Playback;
    m_AppContext.settings = &m_SettingsManager;
    m_AppContext.playlists = &m_Playlists;
    m_AppContext.scanRoots = m_ScanRoots;
    m_AppContext.navigate = [this](std::string_view name) { m_ScreenManager.Push(name); };
    m_AppContext.goBack = [this] { m_ScreenManager.Pop(); };

    m_BackgroundJobs.Start();
    m_Memory.Initialize(
        static_cast<std::size_t>(std::max(0, m_SettingsManager.GetInt("memory.soft_budget_kb", 0))),
        static_cast<std::size_t>(std::max(0, m_SettingsManager.GetInt("memory.hard_budget_kb", 0))));

    m_ScanSubscription = m_EventBus.Subscribe(flachead::events::Type::LibraryScanFinished,
                                              [this](const flachead::events::Event& event) {
                                                  m_Notifications.Push(
                                                      "Library scan finished",
                                                      std::to_string(event.intValue) + " tracks",
                                                      0, "library.scan");
                                              });

    if (!m_ScanRoots.empty())
    {
        m_LibraryService.StartScan(m_ScanRoots);
    }
}

void Application::RegisterScreens()
{
    m_ScreenManager.RegisterFactory("home", [this] {
        return std::make_unique<flachead::shell::AmbientHomeScreen>(m_ShellServices);
    });
    m_ScreenManager.RegisterFactory("launcher", [this] {
        return std::make_unique<flachead::shell::LauncherScreen>(m_ShellServices);
    });
    m_ScreenManager.RegisterFactory("taskoverview", [this] {
        return std::make_unique<flachead::shell::TaskOverviewScreen>(m_ShellServices);
    });
    m_ScreenManager.RegisterFactory("universal_search", [this] {
        return std::make_unique<flachead::shell::UniversalSearchScreen>(m_ShellServices);
    });
    m_ScreenManager.RegisterFactory("settings", [this] {
        return std::make_unique<flachead::shell::SettingsScreen>(m_ShellServices);
    });
    m_ScreenManager.RegisterFactory("system", [this] {
        return std::make_unique<flachead::shell::SystemScreen>(m_ShellServices);
    });

    const flachead::dap::AppContext& context = m_AppContext;
    m_ScreenManager.RegisterFactory("nowplaying", [context] {
        return std::make_unique<flachead::dap::NowPlayingScreen>(context);
    });
    m_ScreenManager.RegisterFactory("queue", [context] {
        return std::make_unique<flachead::dap::QueueScreen>(context);
    });
    m_ScreenManager.RegisterFactory("scan", [context] {
        return std::make_unique<flachead::dap::ScanScreen>(context);
    });
    m_ScreenManager.RegisterFactory("songs", [context] {
        return std::make_unique<flachead::dap::SongsScreen>(context);
    });
    m_ScreenManager.RegisterFactory("albums", [context] {
        return std::make_unique<flachead::dap::AlbumsScreen>(context);
    });
    m_ScreenManager.RegisterFactory("album", [context] {
        return std::make_unique<flachead::dap::AlbumScreen>(context);
    });
    m_ScreenManager.RegisterFactory("artists", [context] {
        return std::make_unique<flachead::dap::ArtistsScreen>(context);
    });
    m_ScreenManager.RegisterFactory("artist", [context] {
        return std::make_unique<flachead::dap::ArtistScreen>(context);
    });
    m_ScreenManager.RegisterFactory("search", [context] {
        return std::make_unique<flachead::dap::SearchScreen>(context);
    });
    m_ScreenManager.RegisterFactory("favorites", [context] {
        return std::make_unique<flachead::dap::FavoritesScreen>(context);
    });
    m_ScreenManager.RegisterFactory("recent", [context] {
        return std::make_unique<flachead::dap::RecentScreen>(context);
    });
    m_ScreenManager.RegisterFactory("playlists", [context] {
        return std::make_unique<flachead::dap::PlaylistsScreen>(context);
    });
    m_ScreenManager.RegisterFactory("playlist", [context] {
        return std::make_unique<flachead::dap::PlaylistScreen>(context);
    });
    m_ScreenManager.RegisterFactory("dapsettings", [context] {
        return std::make_unique<flachead::dap::SettingsScreen>(context);
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
        m_Animations.Tick(deltaSeconds);
        m_AppManager.Tick(deltaSeconds);
        m_AudioService.PollBackendEvents();
        m_Playback.Update(deltaSeconds);
        m_ScreenManager.Update(deltaSeconds);
        m_BackgroundJobs.Update();
        m_Notifications.Update();
        m_Memory.Update();

        bool handled = false;
        m_InputBackend->Poll([this, &handled](const SDL_Event& event) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_TERMINATING)
            {
                m_Running = false;
                handled = true;
                return;
            }
            m_InputManager.HandleEvent(event);
            if (auto* screen = m_ScreenManager.Current())
            {
                handled = screen->HandleEvent(event) || handled;
            }
        });
        m_InputManager.Update();

        const flachead::system::WindowSize size = m_Window.GetSize();
        const bool resized = size.width != lastWidth || size.height != lastHeight;
        if (resized)
        {
            m_InputManager.SetWindowSize(size.width, size.height);
        }
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

    m_Playback.Shutdown();
    m_AudioService.Shutdown();
    m_BackgroundJobs.Shutdown();
    m_LibraryService.WaitForScan();
    m_AppManager.Shutdown();
    m_Resources.Shutdown();
    m_Renderer.Destroy();
    m_FontManager.ReleaseAll();
    m_Window.Destroy();
    m_Database.Close();
    delete m_Canvas;
    m_Canvas = nullptr;
    m_Running = false;
    TTF_Quit();
}
