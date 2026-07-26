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
    using BackHandler   = std::function<void()>;

    void SetLaunchHandler(LaunchHandler handler);
    void SetBackHandler(BackHandler handler);

    void OnEnter() override;
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    void Select(int index);
    void ActivateSelection();
    void DrawStatusBar(flachead::ui::Canvas& canvas, int width);
    void DrawAppGrid(flachead::ui::Canvas& canvas, int width, int height);

    struct AppEntry
    {
        std::string name;
        std::string icon;
        std::string subtitle;
    };

    std::vector<AppEntry> m_Apps{
        {"Music",       "\xe2\x99\xab",  "Library"},   // ♫
        {"Gallery",     "\xe2\x97\xa7",  "Photos"},    // ◧
        {"Video",       "\xe2\x96\xb6",  "Player"},    // ▶
        {"Calculator",  "\xe2\x8c\x97",  "Math"},      // ⌗
        {"Calendar",    "\xe2\x97\xb7",  "Schedule"},  // ◷
        {"Notes",       "\xe2\x9c\x8e",  "Text"},      // ✎
        {"Settings",    "\xe2\x9a\x99",  "System"},    // ⚙
        {"File Browser","\xe2\x96\xa6",  "Files"},     // ▦
        {"Power",       "\xe2\x8f\xbb",  "System"},    // ⏻
    };

    int   m_SelectedIndex{0};
    float m_Pulse{0.0f};
    float m_EnterAnim{0.0f};   // 0→1 on screen enter
    float m_SelectAnim{0.0f};  // bounces on selection change

    LaunchHandler m_OnLaunch;
    BackHandler   m_OnBack;
};