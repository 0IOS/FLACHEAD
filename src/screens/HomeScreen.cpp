#include "HomeScreen.hpp"

#include "../math/Color.hpp"
#include "../math/Rect.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace
{
constexpr int kColumns = 3;
}

void HomeScreen::SetLaunchHandler(LaunchHandler handler)
{
    m_OnLaunch = std::move(handler);
}

void HomeScreen::SetBackHandler(BackHandler handler)
{
    m_OnBack = std::move(handler);
}

void HomeScreen::OnUpdate(float deltaSeconds)
{
    m_Pulse += deltaSeconds * 6.0f;
}

void HomeScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    const auto accent = canvas.ThemeColor("accent", Color{124, 58, 237, 255});
    const auto panel = canvas.ThemeColor("panel", Color{17, 24, 39, 255});
    const auto foreground = canvas.ThemeColor("foreground", Color::White);
    const auto muted = canvas.ThemeColor("muted", Color{148, 163, 184, 255});

    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)}, Color{5, 7, 12, 255});

    canvas.DrawText(Rect{24.0f, 24.0f, 220.0f, 28.0f}, "FLACHEAD", foreground, 24.0f);
    canvas.DrawText(Rect{24.0f, 54.0f, 260.0f, 18.0f}, "Handheld launcher", muted, 16.0f);

    const float padding = std::min(24.0f, width * 0.04f);
    const float cardWidth = (width - padding * 4.0f) / static_cast<float>(kColumns);
    const float cardHeight = 92.0f;

    for (int index = 0; index < static_cast<int>(m_Apps.size()); ++index)
    {
        const int row = index / kColumns;
        const int col = index % kColumns;
        const float x = padding + col * (cardWidth + padding);
        const float y = 110.0f + row * (cardHeight + padding * 0.6f);
        const Rect card{x, y, cardWidth, cardHeight};
        const bool selected = index == m_SelectedIndex;

        const Color cardColor = selected ? accent : panel;
        const Color borderColor = selected ? Color::White : Color{80, 80, 95, 255};
        canvas.FillRect(card, cardColor);
        canvas.DrawRect(card, borderColor);

        const float pulse = selected ? 1.0f + 0.05f * std::sin(m_Pulse) : 1.0f;
        const Rect inner{card.position.x + 10.0f * pulse, card.position.y + 10.0f * pulse,
                         card.size.x - 20.0f * pulse, card.size.y - 20.0f * pulse};
        canvas.DrawRect(inner, selected ? Color::White : Color{120, 120, 130, 255});

        canvas.DrawText(Rect{card.position.x + 14.0f, card.position.y + 20.0f, 40.0f, 24.0f}, m_Icons[index], foreground, 20.0f);
        canvas.DrawText(Rect{card.position.x + 14.0f, card.position.y + 50.0f, card.size.x - 28.0f, 16.0f}, m_Apps[index], foreground, 16.0f);
    }
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
                    {
                        m_OnBack();
                    }
                    return true;
                default:
                    break;
            }
            break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            const float x = static_cast<float>(event.button.x);
            const float y = static_cast<float>(event.button.y);
            const float padding = std::min(24.0f, 900.0f * 0.04f);
            const float cardWidth = (900.0f - padding * 4.0f) / static_cast<float>(kColumns);
            const float cardHeight = 92.0f;
            for (int index = 0; index < static_cast<int>(m_Apps.size()); ++index)
            {
                const int row = index / kColumns;
                const int col = index % kColumns;
                const float cardX = padding + col * (cardWidth + padding);
                const float cardY = 110.0f + row * (cardHeight + padding * 0.6f);
                const Rect card{cardX, cardY, cardWidth, cardHeight};
                if (x >= card.position.x && x <= card.position.x + card.size.x && y >= card.position.y && y <= card.position.y + card.size.y)
                {
                    Select(index);
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
    {
        return;
    }

    m_SelectedIndex = std::clamp(index, 0, static_cast<int>(m_Apps.size() - 1));
}

void HomeScreen::ActivateSelection()
{
    if (m_OnLaunch)
    {
        m_OnLaunch(m_Apps[m_SelectedIndex]);
    }
}
