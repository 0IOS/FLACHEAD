#pragma once

#include "../animation/Animator.hpp"
#include "../app/AppManager.hpp"
#include "../audio/AudioService.hpp"
#include "../core/Logger.hpp"
#include "../core/Renderer.hpp"
#include "../filesystem/FileSystem.hpp"
#include "../input/InputManager.hpp"
#include "../screens/HomeScreen.hpp"
#include "../screens/ScreenManager.hpp"
#include "../services/BatteryManager.hpp"
#include "../services/SettingsManager.hpp"
#include "../services/StorageManager.hpp"
#include "../system/Window.hpp"
#include "../ui/theme/ThemeManager.hpp"
#include "../ui/Canvas.hpp"

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
    flachead::ui::Canvas* m_Canvas{nullptr};
    HomeScreen m_HomeScreen;
    flachead::app::AppManager m_AppManager;
    flachead::screens::ScreenManager m_ScreenManager;
    flachead::theme::ThemeManager m_ThemeManager;
    flachead::input::InputManager m_InputManager;
    flachead::audio::AudioService m_AudioService;
    flachead::filesystem::FileSystem m_FileSystem;
    flachead::services::SettingsManager m_SettingsManager;
    flachead::services::StorageManager m_StorageManager;
    flachead::services::BatteryManager m_BatteryManager;
    flachead::animation::Animator m_Animator;

    bool m_Running{false};
};