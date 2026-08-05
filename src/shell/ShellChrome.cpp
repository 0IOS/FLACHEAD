#include "ShellChrome.hpp"

#include "../math/Color.hpp"

#include <algorithm>
#include <ctime>

namespace flachead::shell
{
namespace
{
constexpr float kStatusHeightRef = 28.0f;
constexpr float kNavHeightRef    = 44.0f;

const Color kStripBg   = Color{12, 15, 23, 205};
const Color kStripLine = Color{30, 36, 51, 255};
const Color kFg        = Color{226, 232, 240, 255};
const Color kFgMuted   = Color{148, 163, 184, 255};
const Color kAccent    = Color{124, 58, 237, 255};
} // namespace

ShellChrome::ShellChrome(const ShellServices& services)
    : m_Services(services)
{
}

float ShellChrome::Scale(int width) const
{
    return std::max(1.0f, static_cast<float>(width) / kReferenceWidth);
}

float ShellChrome::StatusHeight(int width) const
{
    return kStatusHeightRef * Scale(width);
}

float ShellChrome::NavHeight(int width) const
{
    return kNavHeightRef * Scale(width);
}

Rect ShellChrome::ContentArea(int width, int height) const
{
    return Rect{0.0f, StatusHeight(width),
                static_cast<float>(width), static_cast<float>(height) - NavHeight(width) - StatusHeight(width)};
}

float ShellChrome::NavSlotWidth(int width) const
{
    return static_cast<float>(width) / static_cast<float>(sizeof(kNavCommands) / sizeof(kNavCommands[0]));
}

std::string ShellChrome::StatusTime() const
{
    const std::time_t now = std::time(nullptr);
    std::tm timeInfo;
    localtime_r(&now, &timeInfo);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M", &timeInfo);
    return std::string{buf};
}

void ShellChrome::DrawStatus(flachead::ui::Canvas& canvas, int width, int height,
                             std::string_view title) const
{
    (void)height;
    const float statusH = StatusHeight(width);
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), statusH}, kStripBg);
    canvas.DrawLine(0.0f, statusH, static_cast<float>(width), statusH, kStripLine);

    canvas.DrawText(Rect{14.0f, statusH * 0.25f, static_cast<float>(width) * 0.5f, statusH * 0.6f},
                    title, kFg, 13.0f);

    const int battery = m_Services.battery ? m_Services.battery->Percentage() : 100;
    const std::string right = StatusTime() + "  - " + std::to_string(battery) + "%";
    canvas.DrawText(Rect{static_cast<float>(width) * 0.5f, statusH * 0.25f,
                         static_cast<float>(width) * 0.5f - 10.0f, statusH * 0.6f},
                    right, kFgMuted, 12.0f);
}

void ShellChrome::DrawNav(flachead::ui::Canvas& canvas, int width, int height) const
{
    const float navH = NavHeight(width);
    const float y = static_cast<float>(height) - navH;
    const float slotW = NavSlotWidth(width);

    canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), navH}, kStripBg);
    canvas.DrawLine(0.0f, y, static_cast<float>(width), y, kStripLine);

    for (int i = 0; i < 5; ++i)
    {
        const Rect slot{slotW * static_cast<float>(i), y, slotW, navH};
        const bool isHome = kNavCommands[i] == flachead::commands::Command::Home;
        canvas.DrawTextCentered(slot, kNavLabels[i], isHome ? kAccent : kFg, isHome ? 13.0f : 12.0f);
    }
}

bool ShellChrome::HitNavBar(int width, int height, const Vec2& position) const
{
    return position.y >= static_cast<float>(height) - NavHeight(width);
}

flachead::commands::Command ShellChrome::NavCommandAt(int width, int height, const Vec2& position) const
{
    if (!HitNavBar(width, height, position))
    {
        return flachead::commands::Command::None;
    }
    const float slotW = NavSlotWidth(width);
    const int index = std::clamp(static_cast<int>(position.x / slotW), 0, 4);
    return kNavCommands[index];
}
} // namespace flachead::shell
