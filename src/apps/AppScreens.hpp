#pragma once

#include "../screens/ScreenManager.hpp"
#include "../screens/HomeScreen.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace flachead::apps
{
class AppScreen : public flachead::screens::Screen
{
public:
    explicit AppScreen(std::string title)
        : m_Title(std::move(title))
    {
    }

    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

    void SetBackHandler(std::function<void()> handler);

protected:
    std::string m_Title;
    std::function<void()> m_OnBack;
};

class MusicScreen : public AppScreen
{
public:
    MusicScreen();
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    int m_SelectedTrack{0};
    bool m_Playing{true};
    float m_Progress{0.35f};
    float m_Animation{0.0f};
};

class GalleryScreen : public AppScreen
{
public:
    GalleryScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    int m_SelectedIndex{0};
    bool m_Fullscreen{false};
};

class VideoScreen : public AppScreen
{
public:
    VideoScreen();
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    bool m_Playing{true};
    float m_Progress{0.28f};
};

class CalculatorScreen : public AppScreen
{
public:
    CalculatorScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::string m_Display{"0"};
    std::string m_Accumulator;
    std::string m_Operation;
    bool m_ClearNext{false};
};

class CalendarScreen : public AppScreen
{
public:
    CalendarScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    int m_MonthOffset{0};
};

class NotesScreen : public AppScreen
{
public:
    NotesScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::string m_Text{"Meet at dusk\nReview notes\n"};
    int m_Cursor{0};
};

class SettingsScreen : public AppScreen
{
public:
    SettingsScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    int m_SelectedIndex{0};
};

class FileBrowserScreen : public AppScreen
{
public:
    FileBrowserScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::vector<std::string> m_Items{"Music", "Photos", "Videos", "Documents"};
    int m_SelectedIndex{0};
};

class PowerScreen : public AppScreen
{
public:
    PowerScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    int m_SelectedIndex{0};
};
} // namespace flachead::apps
