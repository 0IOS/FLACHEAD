#include "HomeScreen.hpp"

#include "../math/Color.hpp"
#include "../math/Rect.hpp"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <string>

namespace
{
constexpr int   kColumns   = 3;
constexpr float kCardW     = 160.0f;
constexpr float kCardH     = 120.0f;
constexpr float kCardGapX  = 20.0f;
constexpr float kCardGapY  = 18.0f;
constexpr float kGridTop   = 88.0f;
constexpr float kStatusH   = 44.0f;
} // namespace

void HomeScreen::SetLaunchHandler(LaunchHandler handler) { m_OnLaunch = std::move(handler); }
void HomeScreen::SetBackHandler(BackHandler handler)     { m_OnBack  = std::move(handler); }

void HomeScreen::OnEnter()
{
    m_EnterAnim = 0.0f;
}

void HomeScreen::OnUpdate(float deltaSeconds)
{
    m_Pulse      += deltaSeconds * 4.0f;
    m_EnterAnim   = std::min(1.0f, m_EnterAnim + deltaSeconds * 3.5f);
    m_SelectAnim += deltaSeconds * 8.0f;
}

void HomeScreen::DrawStatusBar(flachead::ui::Canvas& canvas, int width)
{
    // Dark translucent strip at top
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), kStatusH},
                    Color{5, 7, 12, 240});

    // FLACHEAD wordmark left
    canvas.DrawText(Rect{20.0f, 10.0f, 160.0f, 24.0f}, "FLACHEAD",
                    Color{241, 245, 249, 255}, 20.0f);

    // Separator line
    canvas.DrawLine(0.0f, kStatusH, static_cast<float>(width), kStatusH,
                    Color{30, 36, 51, 255});

    // Time + battery right
    std::time_t t = std::time(nullptr);
    std::tm* tm_info = std::localtime(&t);
    char timeBuf[16];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M", tm_info);

    canvas.DrawText(Rect{static_cast<float>(width) - 180.0f, 12.0f, 80.0f, 20.0f},
                    timeBuf, Color{200, 210, 220, 255}, 16.0f);

    // Battery pill
    const float bx = static_cast<float>(width) - 84.0f;
    const float by = 13.0f;
    canvas.FillRoundedRect(Rect{bx, by, 60.0f, 18.0f}, 4.0f, Color{20, 26, 38, 255});
    canvas.FillRoundedRect(Rect{bx + 2.0f, by + 2.0f, 46.0f, 14.0f}, 3.0f, Color{34, 211, 238, 255});
    canvas.DrawText(Rect{bx + 6.0f, by + 1.0f, 50.0f, 16.0f}, "82%",
                    Color{5, 7, 12, 255}, 13.0f);
}

void HomeScreen::DrawAppGrid(flachead::ui::Canvas& canvas, int width, int height)
{
    const int   count   = static_cast<int>(m_Apps.size());
    const int   rows    = (count + kColumns - 1) / kColumns;
    const float gridW   = kColumns * kCardW + (kColumns - 1) * kCardGapX;
    const float startX  = (static_cast<float>(width) - gridW) * 0.5f;
    const float easeIn  = m_EnterAnim * m_EnterAnim * (3.0f - 2.0f * m_EnterAnim); // smoothstep

    const Color accent    = Color{124, 58, 237, 255};
    const Color accentGlow= Color{124, 58, 237, 60};
    const Color cardBg    = Color{14, 18, 28, 255};
    const Color cardBgSel = Color{20, 14, 42, 255};
    const Color border    = Color{30, 36, 51, 255};
    const Color fgPrimary = Color{241, 245, 249, 255};
    const Color fgMuted   = Color{100, 116, 139, 255};

    for (int idx = 0; idx < count; ++idx)
    {
        const int   row = idx / kColumns;
        const int   col = idx % kColumns;
        const float cx  = startX + col * (kCardW + kCardGapX);

        // Staggered slide-in from below
        const float delay   = (row * kColumns + col) * 0.04f;
        const float anim    = std::max(0.0f, std::min(1.0f, (m_EnterAnim * 1.5f - delay)));
        const float ease    = anim * anim * (3.0f - 2.0f * anim);
        const float slideY  = (1.0f - ease) * 60.0f;
        const float cy      = kGridTop + static_cast<float>(row) * (kCardH + kCardGapY) + slideY;

        const bool selected = idx == m_SelectedIndex;

        // Glow behind selected card
        if (selected)
        {
            const float glow = 1.0f + 0.04f * std::sin(m_Pulse);
            canvas.FillRoundedRect(Rect{cx - 8.0f * glow, cy - 8.0f * glow,
                                        kCardW + 16.0f * glow, kCardH + 16.0f * glow},
                                   14.0f, accentGlow);
        }

        // Card background
        canvas.FillRoundedRect(Rect{cx, cy, kCardW, kCardH}, 10.0f,
                               selected ? cardBgSel : cardBg);

        // Border
        canvas.DrawRoundedRect(Rect{cx, cy, kCardW, kCardH}, 10.0f,
                               selected ? accent : border);

        if (selected)
        {
            // Double border for selection depth
            canvas.DrawRoundedRect(Rect{cx + 2.0f, cy + 2.0f, kCardW - 4.0f, kCardH - 4.0f},
                                   8.0f, Color{124, 58, 237, 100});
        }

        // Icon
        canvas.DrawText(Rect{cx + 18.0f, cy + 18.0f, 40.0f, 36.0f},
                        m_Apps[idx].icon, fgPrimary, 28.0f);

        // App name
        canvas.DrawText(Rect{cx + 14.0f, cy + 66.0f, kCardW - 28.0f, 22.0f},
                        m_Apps[idx].name, fgPrimary, 15.0f);

        // Subtitle
        canvas.DrawText(Rect{cx + 14.0f, cy + 90.0f, kCardW - 28.0f, 16.0f},
                        m_Apps[idx].subtitle, fgMuted, 12.0f);
    }
}

void HomeScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    // Background
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)},
                    Color{5, 7, 12, 255});

    // Subtle radial-ish gradient flair (two overlapping circles)
    canvas.FillCircle(static_cast<float>(width) * 0.75f, 80.0f, 180.0f,
                      Color{30, 10, 70, 60});
    canvas.FillCircle(static_cast<float>(width) * 0.2f,
                      static_cast<float>(height) - 60.0f, 140.0f,
                      Color{10, 30, 60, 50});

    DrawStatusBar(canvas, width);
    DrawAppGrid(canvas, width, height);

    // Bottom hint
    canvas.DrawText(Rect{0.0f, static_cast<float>(height) - 26.0f, static_cast<float>(width), 20.0f},
                    "ARROWS: navigate   ENTER: open   ESC: exit",
                    Color{40, 50, 70, 255}, 12.0f);
}

bool HomeScreen::HandleEvent(const SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_EVENT_KEY_DOWN:
        {
            switch (event.key.key)
            {
                case SDLK_LEFT:
                    Select(m_SelectedIndex - 1);
                    return true;
                case SDLK_RIGHT:
                    Select(m_SelectedIndex + 1);
                    return true;
                case SDLK_UP:
                    Select(m_SelectedIndex - kColumns);
                    return true;
                case SDLK_DOWN:
                    Select(m_SelectedIndex + kColumns);
                    return true;
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    ActivateSelection();
                    return true;
                case SDLK_ESCAPE:
                    if (m_OnBack)
                        m_OnBack();
                    return true;
                default:
                    break;
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            const float mx = static_cast<float>(event.button.x);
            const float my = static_cast<float>(event.button.y);
            const int   count = static_cast<int>(m_Apps.size());
            const float gridW = kColumns * kCardW + (kColumns - 1) * kCardGapX;
            // Use a reasonable default width for hit testing
            const float startX = (900.0f - gridW) * 0.5f;
            for (int idx = 0; idx < count; ++idx)
            {
                const int   row = idx / kColumns;
                const int   col = idx % kColumns;
                const float cx  = startX + col * (kCardW + kCardGapX);
                const float cy  = kGridTop + static_cast<float>(row) * (kCardH + kCardGapY);
                if (mx >= cx && mx <= cx + kCardW && my >= cy && my <= cy + kCardH)
                {
                    Select(idx);
                    ActivateSelection();
                    return true;
                }
            }
            break;
        }
        default:
            break;
    }
    return false;
}

void HomeScreen::Select(int index)
{
    if (m_Apps.empty())
        return;
    m_SelectedIndex = std::clamp(index, 0, static_cast<int>(m_Apps.size() - 1));
    m_SelectAnim = 0.0f;
}

void HomeScreen::ActivateSelection()
{
    if (m_OnLaunch)
        m_OnLaunch(m_Apps[m_SelectedIndex].name);
}
