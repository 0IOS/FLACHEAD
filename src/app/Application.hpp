#pragma once

#include "../animation/Animator.hpp"
#include "../app/AppManager.hpp"
#include "../audio/AudioService.hpp"
#include "../core/Logger.hpp"
#include "../core/Renderer.hpp"
#include "../database/Database.hpp"
#include "../dap/AppContext.hpp"
#include "../events/EventBus.hpp"
#include "../filesystem/FileSystem.hpp"
#include "../graphics/FontManager.hpp"
#include "../input/GpioInputBackend.hpp"
#include "../input/InputBackend.hpp"
#include "../input/SdlInputBackend.hpp"
#include "../library/LibraryService.hpp"
#include "../playback/PlaybackController.hpp"
#include "../playback/PlaylistEngine.hpp"
#include "../screens/ScreenManager.hpp"
#include "../services/BatteryManager.hpp"
#include "../resource/ResourceManager.hpp"
#include "../models/BatteryStateModel.hpp"
#include "../models/SettingsModel.hpp"
#include "../services/SettingsManager.hpp"
#include "../services/StorageManager.hpp"
#include "../system/Window.hpp"
#include "../ui/theme/ThemeManager.hpp"
#include "../ui/Canvas.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

class Application
{
public:
    explicit Application(float benchmarkSeconds = 0.0f, std::string_view inputBackend = "sdl");
    ~Application();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    void RegisterScreens(const flachead::dap::AppContext& context);
    void SetupServices();
    void RenderFrame(int width, int height);
    void UpdateFrameTier(float renderMs);

    flachead::events::EventBus m_EventBus;
    flachead::system::Window m_Window;
    flachead::core::Renderer m_Renderer;
    flachead::graphics::FontManager m_FontManager;
    flachead::ui::Canvas* m_Canvas{nullptr};
    flachead::app::AppManager m_AppManager;
    flachead::screens::ScreenManager m_ScreenManager;
    flachead::theme::ThemeManager m_ThemeManager;
    flachead::resource::ResourceManager m_Resources;
    flachead::models::BatteryStateModel m_BatteryState;
    flachead::models::SettingsModel m_Settings;
    flachead::input::InputBackend* m_InputBackend{nullptr};
    std::unique_ptr<flachead::input::InputBackend> m_OwnedInputBackend;
    flachead::audio::AudioService m_AudioService;
    flachead::database::Database m_Database;
    flachead::library::LibraryService m_LibraryService;
    flachead::playback::PlaybackController m_Playback;
    flachead::playback::PlaylistEngine m_Playlists;
    flachead::filesystem::FileSystem m_FileSystem;
    flachead::services::SettingsManager m_SettingsManager;
    flachead::services::StorageManager m_StorageManager;
    flachead::services::BatteryManager m_BatteryManager;
    flachead::animation::Animator m_Animator;
    flachead::dap::AppContext m_AppContext;
    std::vector<std::string> m_ScanRoots;

    float m_BenchmarkSeconds{0.0f};
    float m_FpsAccumulator{0.0f};
    std::uint32_t m_FramesThisSecond{0};
    std::uint32_t m_MinFpsSecond{0xFFFFFFFFu};
    std::uint32_t m_MaxFpsSecond{0};
    float m_WorstFrameMs{0.0f};
    float m_TotalFrameMs{0.0f};
    std::uint32_t m_FrameCount{0};

    std::uint32_t m_FrameTier{60};
    float m_FrameTimeEma{0.0f};
    int m_TierStabilityFrames{0};

    bool m_Running{false};
};
