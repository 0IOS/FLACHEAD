#pragma once

#include "../screens/ScreenManager.hpp"
#include "../screens/HomeScreen.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace flachead::apps
{
// ---------------------------------------------------------------------------
// AppScreen — Base class for all application screens
// ---------------------------------------------------------------------------
class AppScreen : public flachead::screens::Screen
{
public:
    explicit AppScreen(std::string title, std::string category = "FLACHEAD OS")
        : m_Title(std::move(title)), m_Category(std::move(category))
    {
    }

    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;
    void SetBackHandler(std::function<void()> handler);

protected:
    void DrawStatusBar(flachead::ui::Canvas& canvas, int width) const;
    void DrawHeader(flachead::ui::Canvas& canvas, int width, const std::string& title,
                    const std::string& subtitle = "") const;
    void DrawFooterHints(flachead::ui::Canvas& canvas, int width, int height,
                         const std::string& hints = "[ARROWS] Navigate  [ENTER] Select  [ESC] Back") const;
    void DrawCard(flachead::ui::Canvas& canvas, const Rect& rect, const std::string& title,
                  const std::string& subtitle, bool selected, const Color& accentColor = Color{124, 58, 237, 255}) const;

    std::string m_Title;
    std::string m_Category;
    std::function<void()> m_OnBack;
};

// ---------------------------------------------------------------------------
// MusicScreen — Flagship Handheld Audio Player
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
    struct Track
    {
        std::string title;
        std::string artist;
        std::string album;
        std::string codec;
        std::string durationStr;
        float       durationSec;
        Color       artColor;
        Color       artAccent;
    };

    void DrawVinylArtwork(flachead::ui::Canvas& canvas, const Rect& rect, float spin, bool playing, const Track& track) const;
    void DrawVisualizer(flachead::ui::Canvas& canvas, const Rect& rect, bool playing) const;

    std::vector<Track> m_Tracks{
        {"Aether Bloom",       "Liora Vale",  "Soft Orbit",     "FLAC 24-bit · 96kHz",  "04:58", 298.0f, Color{40, 20, 80, 255},  Color{168, 85, 247, 255}},
        {"Neon Drift",         "Kade Row",    "Signal Echo",    "FLAC 16-bit · 44.1kHz","03:42", 222.0f, Color{15, 45, 90, 255},  Color{34, 211, 238, 255}},
        {"Midnight Static",    "Mina Sol",    "Night Pulse",    "AAC 320kbps",          "05:11", 311.0f, Color{90, 15, 50, 255},  Color{244, 63, 94, 255}},
        {"Cybernetic Horizon", "Vektor 9",    "Orbital Drift",  "DSD 2.8MHz",           "06:15", 375.0f, Color{20, 60, 40, 255},  Color{52, 211, 153, 255}},
        {"Echoes in Void",     "Eclipse",     "Zero Gravity",   "FLAC 24-bit · 192kHz", "04:20", 260.0f, Color{70, 70, 20, 255},  Color{250, 204, 21, 255}},
    };

    int   m_SelectedTrack{0};
    bool  m_Playing{true};
    float m_Progress{0.32f};      // 0..1
    float m_ElapsedSeconds{95.0f};
    float m_Spin{0.0f};
    float m_PulseAnim{0.0f};
    float m_Volume{0.80f};
    bool  m_Shuffle{false};
    bool  m_Repeat{true};
    std::array<float, 16> m_EqualizerBars{};
};

// ---------------------------------------------------------------------------
// GalleryScreen — Handheld Image Viewer & Grid
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
        std::string title;
        std::string meta;
        std::string camera;
        std::string date;
        Color       bg;
        Color       fg;
        int         patternType;
    };

    void DrawThumbnailPattern(flachead::ui::Canvas& canvas, const Rect& rect, const Photo& photo) const;

    std::vector<Photo> m_Photos{
        {"Sunset Coast",      "4032×3024 · 4.2 MB", "Sony Alpha 7IV", "2026-07-24", Color{180, 70, 20, 255}, Color{255, 160, 50, 255}, 0},
        {"Snow Summit",       "6000×4000 · 8.9 MB", "Leica Q2",       "2026-06-18", Color{30, 80, 150, 255}, Color{140, 210, 255, 255}, 1},
        {"Cyber City",        "3840×2160 · 3.1 MB", "Fujifilm X-T5",  "2026-07-10", Color{80, 20, 90, 255}, Color{236, 72, 153, 255}, 2},
        {"Soundwave Studio",  "4912×3264 · 6.5 MB", "Canon R5",       "2026-05-30", Color{20, 70, 60, 255}, Color{45, 212, 191, 255}, 3},
        {"Architectural Grid","3000×3000 · 2.8 MB", "Hasselblad X2D", "2026-04-12", Color{50, 50, 60, 255}, Color{203, 213, 225, 255}, 4},
        {"Solar Eclipse",     "5472×3648 · 7.4 MB", "Nikon Z9",       "2026-03-08", Color{60, 40, 10, 255}, Color{251, 191, 36, 255}, 5},
    };

    int   m_SelectedIndex{0};
    bool  m_Fullscreen{false};
    float m_FullscreenAnim{0.0f};
};

// ---------------------------------------------------------------------------
// VideoScreen — Handheld Video Player
// ---------------------------------------------------------------------------
class VideoScreen : public AppScreen
{
public:
    VideoScreen();
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    struct VideoItem
    {
        std::string title;
        std::string codec;
        std::string resolution;
        std::string durationStr;
        float       durationSec;
        Color       color;
    };

    std::vector<VideoItem> m_Videos{
        {"FLACHEAD OS Demo",  "H.264 · 60fps", "1920×1080", "02:34", 154.0f, Color{30, 40, 90, 255}},
        {"Handheld Cinema",   "HEVC · 30fps",  "3840×2160", "08:12", 492.0f, Color{80, 20, 60, 255}},
        {"Synthwave Concert", "VP9  · 60fps",  "1920×1080", "12:45", 765.0f, Color{20, 80, 70, 255}},
    };

    int   m_SelectedVideo{0};
    bool  m_Playing{true};
    float m_Progress{0.25f};
    float m_ElapsedSeconds{38.5f};
    float m_AnimTimer{0.0f};
    float m_Volume{0.75f};
};

// ---------------------------------------------------------------------------
// CalculatorScreen — Handheld Math Calculator
// ---------------------------------------------------------------------------
class CalculatorScreen : public AppScreen
{
public:
    CalculatorScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    void InputChar(char ch);
    void InputOp(char op);
    void CalculateResult();
    void ClearAll();
    void Backspace();

    std::string m_Display{"0"};
    std::string m_Expression;
    double      m_Accumulator{0.0};
    char        m_CurrentOp{'\0'};
    bool        m_NewEntry{true};

    int m_FocusedRow{0};
    int m_FocusedCol{0};

    const std::vector<std::vector<std::string>> m_Keypad{
        {"C", "(", ")", "/"},
        {"7", "8", "9", "*"},
        {"4", "5", "6", "-"},
        {"1", "2", "3", "+"},
        {"0", ".", "⌫", "="}
    };
};

// ---------------------------------------------------------------------------
// CalendarScreen — Month Calendar & Agenda
// ---------------------------------------------------------------------------
class CalendarScreen : public AppScreen
{
public:
    CalendarScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    struct EventItem
    {
        int         day;
        std::string time;
        std::string title;
        std::string category;
    };

    static int DaysInMonth(int year, int month);
    static int StartWeekday(int year, int month);

    int m_Year{2026};
    int m_Month{7}; // July
    int m_SelectedDay{26};

    std::vector<EventItem> m_Events{
        {26, "10:00", "FLACHEAD v1.0 Launch", "Release"},
        {26, "14:30", "Audio Engine Testing", "Dev"},
        {26, "19:00", "Community Live Stream", "Media"},
        {28, "11:00", "System Maintenance",  "System"},
        {30, "16:00", "UI Polish Review",     "Design"},
    };
};

// ---------------------------------------------------------------------------
// NotesScreen — Text Scratchpad & Notes Editor
// ---------------------------------------------------------------------------
class NotesScreen : public AppScreen
{
public:
    NotesScreen();
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    struct NoteDoc
    {
        std::string title;
        std::string filename;
        std::string content;
        size_t      cursorPos{0};
    };

    std::vector<NoteDoc> m_Notes{
        {"Quick Ideas",  "01_ideas.txt",  "FLACHEAD Handheld OS\n- Ultra sleek monochrome UI\n- Dynamic audio equalizer\n- Direct Pi boot target\n", 0},
        {"System Log",   "02_syslog.md",  "System status: OK\nMemory: 82% free\nAudio: FLAC 24bit/96kHz active\nGPU: SDL3 hardware renderer\n", 0},
        {"Audio Tracks", "03_playlist.txt","1. Aether Bloom - Liora Vale\n2. Neon Drift - Kade Row\n3. Midnight Static - Mina Sol\n", 0},
    };

    int   m_ActiveNote{0};
    float m_CursorBlink{0.0f};
    bool  m_CursorVisible{true};
    bool  m_Saved{true};
    float m_SavedNotificationTimer{0.0f};
};

// ---------------------------------------------------------------------------
// SettingsScreen — Handheld System Preferences
// ---------------------------------------------------------------------------
class SettingsScreen : public AppScreen
{
public:
    SettingsScreen();
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    enum class ItemType { Toggle, Slider, Picker, Display };

    struct SettingItem
    {
        std::string              name;
        std::string              desc;
        ItemType                 type;
        bool                     toggleVal{false};
        float                    sliderVal{0.5f};
        std::vector<std::string> options;
        size_t                   optionIdx{0};
        std::string              displayVal;
    };

    std::vector<SettingItem> m_Items{
        {"Wi-Fi Network",      "Wireless LAN connection",       ItemType::Toggle,  true,  0.0f,  {}, 0, ""},
        {"Bluetooth",          "Wireless peripherals & audio",  ItemType::Toggle,  false, 0.0f,  {}, 0, ""},
        {"Master Volume",      "Handheld speaker & headphone",  ItemType::Slider,  false, 0.80f, {}, 0, ""},
        {"Screen Brightness",  "Display backlight intensity",   ItemType::Slider,  false, 0.75f, {}, 0, ""},
        {"Audio Profile",      "DSP EQ preset",                 ItemType::Picker,  false, 0.0f,  {"Bit-Perfect", "Bass Boost", "Vocal Clarity", "Flat"}, 0, ""},
        {"Color Accent",       "UI theme palette",              ItemType::Picker,  false, 0.0f,  {"Neon Violet", "Obsidian Cyan", "Crimson Red", "Monochrome"}, 0, ""},
        {"Sleep Timeout",      "Auto display standby",          ItemType::Picker,  false, 0.0f,  {"2 Minutes", "5 Minutes", "10 Minutes", "Never"}, 1, ""},
        {"Battery Status",     "Capacity & charge state",       ItemType::Display, false, 0.0f,  {}, 0, "82% · 4.1V · Discharging"},
        {"Storage Space",      "Internal NVMe memory",          ItemType::Display, false, 0.0f,  {}, 0, "42.8 GB / 64.0 GB Free"},
        {"About Device",       "Hardware & OS version",         ItemType::Display, false, 0.0f,  {}, 0, "FLACHEAD OS v1.0.0 (Linux ARM64)"},
    };

    int m_SelectedIndex{0};
    int m_ScrollOffset{0};
};

// ---------------------------------------------------------------------------
// FileBrowserScreen — Virtual File Manager
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
        std::string sizeStr;
        std::string dateStr;
        bool        isDir;
        std::string icon;
    };

    void NavigateTo(const std::string& path);

    std::string m_CurrentPath{"/storage/media"};

    std::map<std::string, std::vector<FsEntry>> m_FileSystem{
        {"/storage/media", {
            {"..",           "<DIR>",  "2026-07-26", true,  "📁"},
            {"Music",        "<DIR>",  "2026-07-24", true,  "📁"},
            {"Photos",       "<DIR>",  "2026-07-20", true,  "📁"},
            {"Videos",       "<DIR>",  "2026-07-15", true,  "📁"},
            {"Documents",    "<DIR>",  "2026-07-10", true,  "📁"},
            {"README.txt",   "1.2 KB", "2026-07-26", false, "📄"},
        }},
        {"/storage/media/Music", {
            {"..",                   "<DIR>",   "2026-07-26", true,  "📁"},
            {"Aether Bloom.flac",    "34.8 MB", "2026-07-24", false, "🎵"},
            {"Neon Drift.flac",      "28.4 MB", "2026-07-22", false, "🎵"},
            {"Midnight Static.m4a",  "11.2 MB", "2026-07-20", false, "🎵"},
        }},
        {"/storage/media/Photos", {
            {"..",                   "<DIR>",   "2026-07-26", true,  "📁"},
            {"Sunset Coast.jpg",     "4.2 MB",  "2026-07-24", false, "🖼️"},
            {"Snow Summit.raw",      "8.9 MB",  "2026-06-18", false, "🖼️"},
            {"Cyber City.png",       "3.1 MB",  "2026-07-10", false, "🖼️"},
        }},
        {"/storage/media/Videos", {
            {"..",                   "<DIR>",   "2026-07-26", true,  "📁"},
            {"OS Demo.mp4",          "142 MB",  "2026-07-25", false, "🎬"},
            {"Handheld Cinema.mkv",  "850 MB",  "2026-07-18", false, "🎬"},
        }},
        {"/storage/media/Documents", {
            {"..",                   "<DIR>",   "2026-07-26", true,  "📁"},
            {"01_ideas.txt",         "420 B",   "2026-07-26", false, "📄"},
            {"02_syslog.md",         "1.8 KB",  "2026-07-25", false, "📄"},
        }}
    };

    int m_SelectedIndex{0};
    int m_ScrollOffset{0};
};

// ---------------------------------------------------------------------------
// PowerScreen — System Power & Restart Options
// ---------------------------------------------------------------------------
class PowerScreen : public AppScreen
{
public:
    PowerScreen();
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    struct Option
    {
        std::string name;
        std::string desc;
        std::string icon;
        Color       color;
    };

    std::vector<Option> m_Options{
        {"Shutdown",     "Safely turn off FLACHEAD",  "⏻", Color{239, 68, 68, 255}},
        {"Restart",      "Reboot FLACHEAD OS",        "🔄", Color{245, 158, 11, 255}},
        {"Sleep Mode",   "Low power display standby", "🌙", Color{59, 130, 246, 255}},
        {"Cancel",       "Return to launcher",        "❌", Color{148, 163, 184, 255}},
    };

    int         m_SelectedIndex{0};
    bool        m_Triggered{false};
    std::string m_TriggeredAction;
    float       m_Countdown{3.0f};
};

} // namespace flachead::apps
