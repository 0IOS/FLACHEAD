#pragma once

#include "ScreenManager.hpp"
#include "../ui/Canvas.hpp"

#include <functional>
#include <string>
#include <vector>

class HomeScreen : public flachead::screens::Screen
{
public:
    using LaunchHandler = std::function<void(std::string_view)>;
    using BackHandler = std::function<void()>;

    void SetLaunchHandler(LaunchHandler handler);
    void SetBackHandler(BackHandler handler);

    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    void Select(int index);
    void ActivateSelection();

    std::vector<std::string> m_Apps{"Music", "Gallery", "Chess", "Calculator", "Calendar", "Notes", "Video", "Settings", "File Browser", "Power"};
    std::vector<std::string> m_Icons{"♪", "◧", "♞", "⌘", "◷", "✎", "▶", "⚙", "▦", "⏻"};

    int m_SelectedIndex{0};
    float m_Pulse{0.0f};
    LaunchHandler m_OnLaunch;
    BackHandler m_OnBack;
};