#pragma once

#include "../screens/ScreenManager.hpp"
#include "../screens/HomeScreen.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace flachead::apps
{
// ---------------------------------------------------------------------------
// AppScreen — base for all application screens
// ---------------------------------------------------------------------------
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
    void DrawHeader(flachead::ui::Canvas& canvas, int width, const std::string& title,
                    const std::string& subtitle = "") const;
    void DrawStatusBar(flachead::ui::Canvas& canvas, int width) const;
    void DrawBackHint(flachead::ui::Canvas& canvas, int width, int height) const;

    std::string m_Title;
    std::function<void()> m_OnBack;
};

// ---------------------------------------------------------------------------
// MusicScreen
// ---------------------------------------------------------------------------
class MusicScreen : public AppScreen
{
public:
    MusicScreen();
    void OnEnter() override;
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    void DrawAlbumArt(flachead::ui::Canvas& canvas, float x, float y, float size,
                      int trackIndex, float spin, bool playing) const;
    void DrawControls(flachead::ui::Canvas& canvas, float cx, float y, float w, bool playing) const;
    void DrawProgressBar(flachead::ui::Canvas& canvas, float x, float y, float w, float progress) const;

    struct Track
    {
        std::string title;
        std::string artist;
        std::string album;
        std::string codec;
        std::string duration;
        Color       artColor;
        Color       artAccent;
    };

    std::vector<Track> m_Tracks{
        {"Aether Bloom",     "Liora Vale",  "Soft Orbit",   "FLAC 24bit · 96kHz",  "4:58", Color{80,  30, 180, 255}, Color{140, 80, 255, 255}},
        {"Neon Drift",       "Kade Row",    "Signal Echo",  "FLAC 16bit · 44.1kHz", "3:42", Color{20,  80, 160, 255}, Color{40, 180, 255, 255}},
        {"Midnight Static",  "Mina Sol",    "Night Pulse",  "AAC 320kbps",          "5:11", Color{140, 20, 80,  255}, Color{255, 80, 140, 255}},
    };

    int   m_SelectedTrack{0};
    bool  m_Playing{true};
    float m_Progress{0.28f};
    float m_Spin{0.0f};
    float m_PulseAnim{0.0f};
    float m_TrackSwitchAnim{1.0f}; // 0→1 on track change (fade in)
};

// ---------------------------------------------------------------------------
// GalleryScreen
// ---------------------------------------------------------------------------
class GalleryScreen : public AppScreen
{
public:
    GalleryScreen();
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    struct Photo
    {
        std::string name;
        std::string meta;
        Color       color;
        Color       accent;
    };

    std::vector<Photo> m_Photos{
        {"Sunset at Shore",   "4032×3024 · JPEG",  Color{180, 80,  20,  255}, Color{255, 140, 40, 255}},
        {"Snow Mountains",    "6000×4000 · RAW",   Color{60,  100, 180, 255}, Color{130, 200, 255, 255}},
        {"Studio Session",    "3840×2160 · TIFF",  Color{60,  60,  60,  255}, Color{200, 200, 200, 255}},
        {"City Lights",       "4912×3264 · JPEG",  Color{20,  20,  80,  255}, Color{80,  120, 220, 255}},
        {"Forest Path",       "5472×3648 · JPEG",  Color{30,  90,  30,  255}, Color{80,  180, 80,  255}},
        {"Abstract No.7",     "3000×3000 · PNG",   Color{100, 20,  100, 255}, Color{220, 80,  220, 255}},
    };

    int   m_SelectedIndex{0};
    bool  m_Fullscreen{false};
    float m_FullscreenAnim{0.0f};
};

// ---------------------------------------------------------------------------
// VideoScreen
// ---------------------------------------------------------------------------
class VideoScreen : public AppScreen
{
public:
    VideoScreen();
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    struct Video
    {
        std::string title;
        std::string info;
        std::string duration;
        Color       color;
    };

    std::vector<Video> m_Videos{
        {"Intro Reel",    "H.264 · 1920×1080 · 24fps", "2:34", Color{20, 40, 80, 255}},
        {"Studio Tour",   "HEVC · 3840×2160 · 30fps",  "8:12", Color{40, 20, 80, 255}},
        {"Live Set 001",  "VP9  · 1920×1080 · 60fps",  "1:22:05", Color{20, 60, 50, 255}},
    };

    int   m_SelectedVideo{0};
    bool  m_Playing{false};
    float m_Progress{0.0f};
    float m_Noise{0.0f}; // film grain animation counter
    float m_Elapsed{0.0f};
};

// ---------------------------------------------------------------------------
// CalculatorScreen
// ---------------------------------------------------------------------------
class CalculatorScreen : public AppScreen
{
public:
    CalculatorScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    void PressButton(const std::string& label);
    double Evaluate() const;

    std::string m_Display{"0"};
    std::string m_Accumulator;
    std::string m_Operation;
    bool        m_ClearNext{false};
    bool        m_HasDecimal{false};
    int         m_PressedBtn{-1};
    float       m_PressTimer{0.0f};
};

// ---------------------------------------------------------------------------
// CalendarScreen
// ---------------------------------------------------------------------------
class CalendarScreen : public AppScreen
{
public:
    CalendarScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    int m_MonthOffset{0};

    static int DaysInMonth(int year, int month);
    static int StartWeekday(int year, int month);
};

// ---------------------------------------------------------------------------
// NotesScreen
// ---------------------------------------------------------------------------
class NotesScreen : public AppScreen
{
public:
    NotesScreen();
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::vector<std::string> WrapLines(int maxChars) const;

    std::string m_Text{"Meet at dusk\nReview the notes for project\n"};
    float       m_CursorBlink{0.0f};
    bool        m_CursorVisible{true};
    bool        m_Saved{false};
    float       m_SavedTimer{0.0f};
};

// ---------------------------------------------------------------------------
// SettingsScreen
// ---------------------------------------------------------------------------
class SettingsScreen : public AppScreen
{
public:
    SettingsScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    enum class ItemType { Toggle, Slider, Display, Picker };

    struct SettingItem
    {
        std::string name;
        std::string description;
        ItemType    type;
        bool        toggleValue{false};
        float       sliderValue{0.5f};
        std::string displayValue;
    };

    std::vector<SettingItem> m_Items{
        {"Wi-Fi",             "Network connection",    ItemType::Toggle,  true,  0.0f,  ""},
        {"Bluetooth",         "Wireless devices",      ItemType::Toggle,  false, 0.0f,  ""},
        {"Volume",            "Output level",          ItemType::Slider,  false, 0.60f, ""},
        {"Brightness",        "Display brightness",    ItemType::Slider,  false, 0.75f, ""},
        {"Battery",           "Charge status",         ItemType::Display, false, 0.0f,  "82% · Charging"},
        {"Theme",             "Color scheme",          ItemType::Picker,  false, 0.0f,  "Midnight"},
        {"About",             "FLACHEAD v0.1.0",       ItemType::Display, false, 0.0f,  "Handheld OS"},
    };

    int m_SelectedIndex{0};
    int m_ScrollOffset{0};
};

// ---------------------------------------------------------------------------
// FileBrowserScreen
// ---------------------------------------------------------------------------
class FileBrowserScreen : public AppScreen
{
public:
    FileBrowserScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    struct FsEntry
    {
        std::string name;
        std::string meta;
        bool        isDir;
    };

    std::vector<FsEntry> m_Items{
        {"Music",     "Folder · 142 items",  true},
        {"Photos",    "Folder · 87 items",   true},
        {"Videos",    "Folder · 23 items",   true},
        {"Documents", "Folder · 55 items",   true},
        {"Downloads", "Folder · 12 items",   true},
    };

    int m_SelectedIndex{0};
};

// ---------------------------------------------------------------------------
// PowerScreen
// ---------------------------------------------------------------------------
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
