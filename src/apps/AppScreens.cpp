#include "AppScreens.hpp"

#include "../math/Color.hpp"
#include "../math/Rect.hpp"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <vector>

namespace flachead::apps
{
namespace
{
constexpr float kCardHeight = 58.0f;

void DrawHeader(flachead::ui::Canvas& canvas, int width, const std::string& title)
{
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), 70.0f}, Color{9, 12, 18, 255});
    canvas.DrawText(Rect{18.0f, 22.0f, 220.0f, 24.0f}, title, Color::White, 24.0f);
    canvas.DrawText(Rect{18.0f, 46.0f, 260.0f, 16.0f}, "FLACHEAD handheld", Color{140, 140, 150, 255}, 15.0f);
}

void DrawCard(flachead::ui::Canvas& canvas, const Rect& rect, const std::string& title, const std::string& subtitle, bool selected)
{
    const Color bg = selected ? Color{124, 58, 237, 255} : Color{20, 24, 32, 255};
    const Color border = selected ? Color::White : Color{90, 90, 100, 255};
    canvas.FillRect(rect, bg);
    canvas.DrawRect(rect, border);
    canvas.DrawText(Rect{rect.position.x + 14.0f, rect.position.y + 12.0f, rect.size.x - 20.0f, 20.0f}, title, Color::White, 18.0f);
    canvas.DrawText(Rect{rect.position.x + 14.0f, rect.position.y + 34.0f, rect.size.x - 20.0f, 16.0f}, subtitle, Color{220, 220, 220, 255}, 14.0f);
}
} // namespace

void AppScreen::SetBackHandler(std::function<void()> handler)
{
    m_OnBack = std::move(handler);
}

void AppScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)}, Color{6, 8, 13, 255});
    DrawHeader(canvas, width, m_Title);
}

bool AppScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE && m_OnBack)
    {
        m_OnBack();
        return true;
    }

    return false;
}

MusicScreen::MusicScreen()
    : AppScreen("Music")
{
}

void MusicScreen::OnUpdate(float deltaSeconds)
{
    m_Animation += deltaSeconds * 3.0f;
}

void MusicScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    const std::vector<std::string> songs{"Aether Bloom", "Neon Drift", "Midnight Static"};
    const std::vector<std::string> artists{"Liora Vale", "Kade Row", "Mina Sol"};
    const std::vector<std::string> albums{"Soft Orbit", "Signal Echo", "Night Pulse"};

    const Rect art{24.0f, 90.0f, 170.0f, 170.0f};
    canvas.FillRect(art, Color{24, 24, 28, 255});
    canvas.DrawRect(art, Color::White);
    canvas.DrawText(Rect{art.position.x + 20.0f, art.position.y + 72.0f, 120.0f, 24.0f}, "♪", Color::White, 40.0f);

    canvas.DrawText(Rect{210.0f, 96.0f, 220.0f, 24.0f}, songs[m_SelectedTrack], Color::White, 22.0f);
    canvas.DrawText(Rect{210.0f, 124.0f, 220.0f, 18.0f}, artists[m_SelectedTrack], Color{190, 190, 200, 255}, 18.0f);
    canvas.DrawText(Rect{210.0f, 146.0f, 220.0f, 18.0f}, albums[m_SelectedTrack], Color{140, 140, 150, 255}, 16.0f);
    canvas.DrawText(Rect{210.0f, 172.0f, 220.0f, 18.0f}, "AAC 24-bit • 48 kHz", Color{120, 120, 130, 255}, 15.0f);

    const Rect progress{24.0f, 280.0f, width - 48.0f, 12.0f};
    canvas.FillRect(progress, Color{30, 32, 40, 255});
    const float fillWidth = progress.size.x * m_Progress;
    canvas.FillRect(Rect{progress.position.x, progress.position.y, fillWidth, progress.size.y}, Color{255, 255, 255, 255});

    canvas.DrawText(Rect{24.0f, 302.0f, 80.0f, 16.0f}, "03:12", Color::White, 15.0f);
    canvas.DrawText(Rect{width - 92.0f, 302.0f, 80.0f, 16.0f}, "04:58", Color{170, 170, 180, 255}, 15.0f);

    const Rect controls{24.0f, 340.0f, width - 48.0f, 60.0f};
    canvas.FillRect(controls, Color{12, 14, 20, 255});
    canvas.DrawRect(controls, Color{80, 80, 90, 255});
    canvas.DrawText(Rect{controls.position.x + 24.0f, controls.position.y + 20.0f, 36.0f, 20.0f}, "◀", Color::White, 18.0f);
    const Rect play{controls.position.x + (controls.size.x * 0.5f) - 20.0f, controls.position.y + 10.0f, 40.0f, 40.0f};
    canvas.DrawRect(play, Color::White);
    canvas.DrawText(Rect{play.position.x + 12.0f, play.position.y + 10.0f, 20.0f, 20.0f}, m_Playing ? "▮▮" : "▶", Color{8, 8, 12, 255}, 16.0f);
    canvas.DrawText(Rect{controls.position.x + controls.size.x - 80.0f, controls.position.y + 20.0f, 36.0f, 20.0f}, "▶", Color::White, 18.0f);

    canvas.DrawText(Rect{24.0f, height - 42.0f, 220.0f, 16.0f}, "Now playing • Press left/right to change", Color{180, 180, 190, 255}, 14.0f);
}

bool MusicScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_LEFT:
                m_SelectedTrack = (m_SelectedTrack + 2) % 3;
                return true;
            case SDLK_RIGHT:
                m_SelectedTrack = (m_SelectedTrack + 1) % 3;
                return true;
            case SDLK_SPACE:
                m_Playing = !m_Playing;
                return true;
            default:
                break;
        }
    }

    return false;
}

GalleryScreen::GalleryScreen()
    : AppScreen("Gallery")
{
}

void GalleryScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    std::vector<std::string> items{"Sunset", "Snow", "Studio"};
    const float spacing = 12.0f;
    const float cardWidth = (width - 48.0f - spacing * 2.0f) / 3.0f;
    for (int index = 0; index < 3; ++index)
    {
        const Rect rect{24.0f + index * (cardWidth + spacing), 100.0f, cardWidth, 120.0f};
        const bool selected = index == m_SelectedIndex;
        DrawCard(canvas, rect, items[index], selected ? "Open fullscreen" : "Tap to open", selected);
    }

    if (m_Fullscreen)
    {
        canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)}, Color{0, 0, 0, 220});
        canvas.DrawText(Rect{32.0f, 120.0f, width - 64.0f, 40.0f}, "Fullscreen image preview", Color::White, 28.0f);
        canvas.DrawText(Rect{32.0f, 170.0f, width - 64.0f, 20.0f}, "Press Esc to return", Color{180, 180, 190, 255}, 16.0f);
    }
}

bool GalleryScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_LEFT:
                m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
                return true;
            case SDLK_RIGHT:
                m_SelectedIndex = std::min(2, m_SelectedIndex + 1);
                return true;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                m_Fullscreen = !m_Fullscreen;
                return true;
            default:
                break;
        }
    }

    return false;
}

VideoScreen::VideoScreen()
    : AppScreen("Video")
{
}

void VideoScreen::OnUpdate(float deltaSeconds)
{
    (void)deltaSeconds;
}

void VideoScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);
    const Rect video{24.0f, 92.0f, width - 48.0f, 220.0f};
    canvas.FillRect(video, Color{18, 22, 28, 255});
    canvas.DrawRect(video, Color::White);
    canvas.DrawText(Rect{video.position.x + 16.0f, video.position.y + 16.0f, 220.0f, 20.0f}, "Cinema mode", Color::White, 18.0f);
    canvas.DrawText(Rect{video.position.x + 16.0f, video.position.y + 44.0f, 220.0f, 18.0f}, "Handheld video preview", Color{180, 180, 190, 255}, 15.0f);

    const Rect timeline{24.0f, 330.0f, width - 48.0f, 12.0f};
    canvas.FillRect(timeline, Color{26, 30, 38, 255});
    canvas.FillRect(Rect{timeline.position.x, timeline.position.y, timeline.size.x * m_Progress, timeline.size.y}, Color::White);

    const Rect controls{24.0f, 352.0f, width - 48.0f, 54.0f};
    canvas.FillRect(controls, Color{12, 14, 20, 255});
    canvas.DrawRect(controls, Color{80, 80, 90, 255});
    canvas.DrawText(Rect{controls.position.x + 20.0f, controls.position.y + 16.0f, 34.0f, 20.0f}, m_Playing ? "▮▮" : "▶", Color::White, 18.0f);
    canvas.DrawText(Rect{controls.position.x + controls.size.x - 74.0f, controls.position.y + 16.0f, 64.0f, 20.0f}, "01:12", Color{170, 170, 180, 255}, 15.0f);
}

bool VideoScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE)
    {
        m_Playing = !m_Playing;
        return true;
    }

    return false;
}

CalculatorScreen::CalculatorScreen()
    : AppScreen("Calculator")
{
}

void CalculatorScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);
    const Rect display{24.0f, 90.0f, width - 48.0f, 56.0f};
    canvas.FillRect(display, Color{14, 18, 24, 255});
    canvas.DrawRect(display, Color{90, 90, 100, 255});
    canvas.DrawText(Rect{display.position.x + 16.0f, display.position.y + 14.0f, display.size.x - 24.0f, 24.0f}, m_Display, Color::White, 24.0f);

    const std::vector<std::string> labels{"7", "8", "9", "/", "4", "5", "6", "*", "1", "2", "3", "-", "0", ".", "=", "+"};
    const float pad = 8.0f;
    const float btnW = (width - 48.0f - pad * 3.0f) / 4.0f;
    const float btnH = 44.0f;
    for (int index = 0; index < static_cast<int>(labels.size()); ++index)
    {
        const int row = index / 4;
        const int col = index % 4;
        const Rect rect{24.0f + col * (btnW + pad), 162.0f + row * (btnH + pad), btnW, btnH};
        canvas.FillRect(rect, Color{24, 27, 34, 255});
        canvas.DrawRect(rect, Color{120, 120, 130, 255});
        canvas.DrawText(Rect{rect.position.x + 10.0f, rect.position.y + 13.0f, rect.size.x - 20.0f, 20.0f}, labels[index], Color::White, 18.0f);
    }
}

bool CalculatorScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        const auto key = event.key.key;
        if (key >= SDLK_0 && key <= SDLK_9)
        {
            const char digit = static_cast<char>('0' + (key - SDLK_0));
            if (m_ClearNext)
            {
                m_Display = std::string(1, digit);
                m_ClearNext = false;
            }
            else if (m_Display == "0")
            {
                m_Display = std::string(1, digit);
            }
            else
            {
                m_Display += digit;
            }
            return true;
        }

        if (key == SDLK_PLUS || key == SDLK_MINUS || key == SDLK_ASTERISK || key == SDLK_SLASH)
        {
            m_Accumulator = m_Display;
            m_Operation = (key == SDLK_PLUS) ? "+" : (key == SDLK_MINUS) ? "-" : (key == SDLK_ASTERISK) ? "*" : "/";
            m_ClearNext = true;
            return true;
        }

        if (key == SDLK_EQUALS || key == SDLK_RETURN)
        {
            if (m_Operation == "+")
            {
                m_Display = std::to_string(std::stoi(m_Accumulator) + std::stoi(m_Display));
            }
            else if (m_Operation == "-")
            {
                m_Display = std::to_string(std::stoi(m_Accumulator) - std::stoi(m_Display));
            }
            else if (m_Operation == "*")
            {
                m_Display = std::to_string(std::stoi(m_Accumulator) * std::stoi(m_Display));
            }
            else if (m_Operation == "/")
            {
                const auto divisor = std::stoi(m_Display);
                if (divisor != 0)
                {
                    m_Display = std::to_string(std::stoi(m_Accumulator) / divisor);
                }
            }
            m_ClearNext = false;
            return true;
        }
    }

    return false;
}

CalendarScreen::CalendarScreen()
    : AppScreen("Calendar")
{
}

void CalendarScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);
    const Rect month{24.0f, 92.0f, width - 48.0f, 48.0f};
    canvas.FillRect(month, Color{24, 27, 34, 255});
    canvas.DrawRect(month, Color{120, 120, 130, 255});
    canvas.DrawText(Rect{month.position.x + 12.0f, month.position.y + 14.0f, 200.0f, 20.0f}, "July 2026", Color::White, 18.0f);

    const std::vector<std::string> days{"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
    for (int index = 0; index < 7; ++index)
    {
        const Rect rect{24.0f + index * 44.0f, 152.0f, 40.0f, 24.0f};
        canvas.DrawText(Rect{rect.position.x, rect.position.y, 40.0f, 16.0f}, days[index], Color{180, 180, 190, 255}, 14.0f);
    }

    for (int day = 1; day <= 31; ++day)
    {
        const int row = (day - 1) / 7;
        const int col = (day - 1) % 7;
        const Rect cell{24.0f + col * 44.0f, 182.0f + row * 36.0f, 40.0f, 28.0f};
        const bool today = day == 26;
        canvas.FillRect(cell, today ? Color{124, 58, 237, 255} : Color{14, 18, 24, 255});
        canvas.DrawRect(cell, Color{90, 90, 100, 255});
        canvas.DrawText(Rect{cell.position.x + 12.0f, cell.position.y + 6.0f, 20.0f, 16.0f}, std::to_string(day), today ? Color::White : Color{190, 190, 200, 255}, 14.0f);
    }
}

bool CalendarScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        if (event.key.key == SDLK_LEFT)
        {
            m_MonthOffset = std::max(-3, m_MonthOffset - 1);
            return true;
        }
        if (event.key.key == SDLK_RIGHT)
        {
            m_MonthOffset = std::min(3, m_MonthOffset + 1);
            return true;
        }
    }

    return false;
}

NotesScreen::NotesScreen()
    : AppScreen("Notes")
{
}

void NotesScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);
    const Rect editor{24.0f, 90.0f, width - 48.0f, height - 130.0f};
    canvas.FillRect(editor, Color{20, 24, 32, 255});
    canvas.DrawRect(editor, Color{90, 90, 100, 255});
    canvas.DrawText(Rect{editor.position.x + 12.0f, editor.position.y + 12.0f, editor.size.x - 24.0f, 20.0f}, m_Text, Color::White, 16.0f);
    canvas.DrawRect(Rect{editor.position.x + 12.0f, editor.position.y + 42.0f, 8.0f, 18.0f}, Color::White);
}

bool NotesScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_TEXT_INPUT)
    {
        m_Text += event.text.text;
        m_Cursor += 1;
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_BACKSPACE && !m_Text.empty())
    {
        m_Text.pop_back();
        return true;
    }

    return false;
}

SettingsScreen::SettingsScreen()
    : AppScreen("Settings")
{
}

void SettingsScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);
    const std::vector<std::string> items{"Wi-Fi", "Sound", "Battery", "Theme"};
    const std::vector<std::string> values{"On", "50%", "82%", "Midnight"};
    for (int index = 0; index < static_cast<int>(items.size()); ++index)
    {
        const Rect rect{24.0f, 96.0f + index * 64.0f, width - 48.0f, 48.0f};
        const bool selected = index == m_SelectedIndex;
        DrawCard(canvas, rect, items[index], values[index], selected);
    }
}

bool SettingsScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_UP:
                m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
                return true;
            case SDLK_DOWN:
                m_SelectedIndex = std::min(3, m_SelectedIndex + 1);
                return true;
            default:
                break;
        }
    }

    return false;
}

FileBrowserScreen::FileBrowserScreen()
    : AppScreen("File Browser")
{
}

void FileBrowserScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);
    for (int index = 0; index < static_cast<int>(m_Items.size()); ++index)
    {
        const Rect rect{24.0f, 96.0f + index * 54.0f, width - 48.0f, 42.0f};
        const bool selected = index == m_SelectedIndex;
        DrawCard(canvas, rect, m_Items[index], index == 0 ? "Accessible" : "Folder", selected);
    }
}

bool FileBrowserScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_UP:
                m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
                return true;
            case SDLK_DOWN:
                m_SelectedIndex = std::min(static_cast<int>(m_Items.size()) - 1, m_SelectedIndex + 1);
                return true;
            default:
                break;
        }
    }

    return false;
}

PowerScreen::PowerScreen()
    : AppScreen("Power")
{
}

void PowerScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);
    const std::vector<std::string> items{"Shutdown", "Restart", "Sleep", "Cancel"};
    for (int index = 0; index < static_cast<int>(items.size()); ++index)
    {
        const Rect rect{24.0f, 100.0f + index * 56.0f, width - 48.0f, 42.0f};
        const bool selected = index == m_SelectedIndex;
        DrawCard(canvas, rect, items[index], index == 3 ? "Return to launcher" : "Dummy action", selected);
    }
}

bool PowerScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_UP:
                m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
                return true;
            case SDLK_DOWN:
                m_SelectedIndex = std::min(3, m_SelectedIndex + 1);
                return true;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                if (m_SelectedIndex == 3 && m_OnBack)
                {
                    m_OnBack();
                }
                return true;
            default:
                break;
        }
    }

    return false;
}
} // namespace flachead::apps
