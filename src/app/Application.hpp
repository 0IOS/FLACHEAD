#pragma once

#include "../animation/Animator.hpp"
#include "../app/AppManager.hpp"
#include "../audio/AudioService.hpp"
#include "../core/Logger.hpp"
#include "../core/Renderer.hpp"
#include "../filesystem/FileSystem.hpp"
#include "../graphics/FontManager.hpp"
#include "../input/InputManager.hpp"
#include "../screens/HomeScreen.hpp"
#include "../screens/ScreenManager.hpp"
#include "../services/BatteryManager.hpp"
#include "../resource/ResourceManager.hpp"
#include "../models/BatteryStateModel.hpp"
#include "../models/SettingsModel.hpp"
#include "../apps/AppScreens.hpp"
#include "../services/SettingsManager.hpp"
#include "../services/StorageManager.hpp"
#include "../system/Window.hpp"
#include "../ui/theme/ThemeManager.hpp"
#include "../ui/Canvas.hpp"

#include <memory>

class Application
{
public:
    Application();
    ~Application();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    flachead::system::Window m_Window;
    flachead::core::Renderer m_Renderer;
    flachead::graphics::FontManager m_FontManager;
    flachead::ui::Canvas* m_Canvas{nullptr};
    std::unique_ptr<HomeScreen> m_LauncherScreen;
    std::unique_ptr<flachead::apps::MusicScreen> m_MusicScreen;
    std::unique_ptr<flachead::apps::GalleryScreen> m_GalleryScreen;
    std::unique_ptr<flachead::apps::VideoScreen> m_VideoScreen;
    std::unique_ptr<flachead::apps::CalculatorScreen> m_CalculatorScreen;
    std::unique_ptr<flachead::apps::CalendarScreen> m_CalendarScreen;
    std::unique_ptr<flachead::apps::NotesScreen> m_NotesScreen;
    std::unique_ptr<flachead::apps::SettingsScreen> m_SettingsScreen;
    std::unique_ptr<flachead::apps::FileBrowserScreen> m_FileBrowserScreen;
    std::unique_ptr<flachead::apps::PowerScreen> m_PowerScreen;
    flachead::app::AppManager m_AppManager;
    flachead::screens::ScreenManager m_ScreenManager;
    flachead::theme::ThemeManager m_ThemeManager;
    flachead::resource::ResourceManager m_Resources;
    flachead::models::BatteryStateModel m_BatteryState;
    flachead::models::SettingsModel m_Settings;
    flachead::input::InputManager m_InputManager;
    flachead::audio::AudioService m_AudioService;
    flachead::filesystem::FileSystem m_FileSystem;
    flachead::services::SettingsManager m_SettingsManager;
    flachead::services::StorageManager m_StorageManager;
    flachead::services::BatteryManager m_BatteryManager;
    flachead::animation::Animator m_Animator;

    bool m_Running{false};
};