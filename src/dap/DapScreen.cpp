#include "DapScreen.hpp"

#include <algorithm>
#include <cstdio>
#include <ctime>

namespace flachead::dap
{
namespace
{
const Color kBg        = Color{8, 10, 15, 255};
const Color kStripBg   = Color{12, 15, 23, 255};
const Color kLine      = Color{30, 36, 51, 255};
const Color kFg        = Color{226, 232, 240, 255};
const Color kFgMuted   = Color{148, 163, 184, 255};
const Color kAccent    = Color{124, 58, 237, 255};
const Color kRowSel    = Color{28, 35, 51, 255};
const Color kRowBg     = Color{16, 20, 30, 255};
constexpr float kRowH  = 52.0f;
constexpr float kListTop = kStatusH + kHeaderH + 6.0f;
} // namespace

DapScreen::DapScreen(const AppContext& context, std::string title)
    : m_Ctx(context), m_Title(std::move(title))
{
}

DapScreen::~DapScreen()
{
    for (const int id : m_Subscriptions)
    {
        m_Ctx.eventBus->Unsubscribe(id);
    }
}

void DapScreen::OnEnter()
{
    m_NeedsRender = true;
    m_DataDirty = true;
}

void DapScreen::OnUpdate(float)
{
    if (m_DataDirty)
    {
        m_DataDirty = false;
        RefreshData();
        m_NeedsRender = true;
    }
}

bool DapScreen::NeedsRender() const
{
    return m_NeedsRender;
}

int DapScreen::Subscribe(flachead::events::Type type)
{
    return Subscribe(type, [](const flachead::events::Event&) {});
}

int DapScreen::Subscribe(flachead::events::Type type,
                         flachead::events::EventBus::Handler handler)
{
    const int id = m_Ctx.eventBus->Subscribe(type, [this, handler](const flachead::events::Event& e) {
        m_NeedsRender = true;
        m_DataDirty = true;
        if (handler)
        {
            handler(e);
        }
    });
    m_Subscriptions.push_back(id);
    return id;
}

void DapScreen::DrawBackground(flachead::ui::Canvas& canvas, int width, int height) const
{
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)}, kBg);
}

void DapScreen::DrawStatusBar(flachead::ui::Canvas& canvas, int width) const
{
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), kStatusH}, kStripBg);
    canvas.DrawLine(0.0f, kStatusH, static_cast<float>(width), kStatusH, kLine);

    canvas.DrawText(Rect{16.0f, 8.0f, 130.0f, 20.0f}, "FLACHEAD", Color{248, 250, 252, 255}, 16.0f);

    std::time_t t = std::time(nullptr);
    std::tm* tm_info = std::localtime(&t);
    char timeBuf[16];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M", tm_info);
    canvas.DrawText(Rect{static_cast<float>(width) - 80.0f, 10.0f, 60.0f, 18.0f}, timeBuf,
                    Color{203, 213, 225, 255}, 14.0f);
}

void DapScreen::DrawHeader(flachead::ui::Canvas& canvas, int width, const std::string& title,
                           const std::string& subtitle) const
{
    const float y = kStatusH;
    canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), kHeaderH}, kStripBg);
    canvas.DrawLine(0.0f, y + kHeaderH, static_cast<float>(width), y + kHeaderH, kLine);

    canvas.DrawText(Rect{20.0f, y + 8.0f, 340.0f, 24.0f}, title, Color::White, 20.0f);
    if (!subtitle.empty())
    {
        canvas.DrawText(Rect{370.0f, y + 12.0f, 240.0f, 18.0f}, subtitle, kFgMuted, 13.0f);
    }
}

void DapScreen::DrawFooter(flachead::ui::Canvas& canvas, int width, int height,
                           const std::string& hints) const
{
    const float fy = static_cast<float>(height) - kFooterH;
    canvas.FillRect(Rect{0.0f, fy, static_cast<float>(width), kFooterH}, kStripBg);
    canvas.DrawLine(0.0f, fy, static_cast<float>(width), fy, kLine);
    canvas.DrawText(Rect{20.0f, fy + 5.0f, static_cast<float>(width) - 40.0f, 16.0f}, hints,
                    Color{100, 116, 139, 255}, 12.0f);
}

void DapScreen::DrawEmpty(flachead::ui::Canvas& canvas, int width, int height,
                          const std::string& message) const
{
    canvas.DrawTextCentered(
        Rect{0.0f, static_cast<float>(height) * 0.4f, static_cast<float>(width), 24.0f},
        message, kFgMuted, 16.0f);
}

void DapScreen::DrawRow(flachead::ui::Canvas& canvas, int width, float y, float rowH,
                        const std::string& primary, const std::string& secondary,
                        bool selected) const
{
    canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), rowH}, selected ? kRowSel : kRowBg);
    if (selected)
    {
        canvas.FillRect(Rect{0.0f, y, 4.0f, rowH}, kAccent);
    }
    canvas.DrawLine(0.0f, y + rowH, static_cast<float>(width), y + rowH, kLine);

    canvas.DrawText(Rect{22.0f, y + 8.0f, static_cast<float>(width) - 200.0f, 20.0f}, primary,
                    selected ? Color::White : kFg, 16.0f);
    if (!secondary.empty())
    {
        canvas.DrawText(Rect{22.0f, y + 28.0f, static_cast<float>(width) - 220.0f, 16.0f},
                        secondary, kFgMuted, 13.0f);
    }
}

void DapScreen::DrawNowPlayingBar(flachead::ui::Canvas& canvas, int width, int height) const
{
    const auto& playback = *m_Ctx.playback;
    if (playback.Queue().IsEmpty())
    {
        return;
    }

    const float barH = 58.0f;
    const float by = static_cast<float>(height) - kFooterH - barH;
    canvas.FillRect(Rect{0.0f, by, static_cast<float>(width), barH}, kStripBg);
    canvas.DrawLine(0.0f, by, static_cast<float>(width), by, kLine);

    const auto& track = playback.CurrentTrack();
    const std::string title = track.Valid() ? track.DisplayTitle() : "Nothing playing";
    const std::string sub = track.Valid() ? track.DisplayArtist() : "Pick a track";

    canvas.DrawText(Rect{24.0f, by + 8.0f, static_cast<float>(width) - 170.0f, 20.0f}, title,
                    Color::White, 16.0f);
    canvas.DrawText(Rect{24.0f, by + 30.0f, static_cast<float>(width) - 170.0f, 16.0f}, sub,
                    kFgMuted, 13.0f);

    canvas.DrawText(Rect{static_cast<float>(width) - 120.0f, by + 18.0f, 90.0f, 20.0f},
                    playback.IsPlaying() ? "▶ Playing" : "⏸ Paused",
                    playback.IsPlaying() ? Color{52, 211, 153, 255} : Color{245, 158, 11, 255},
                    14.0f);
}

std::string DapScreen::FormatDuration(double seconds)
{
    const int total = std::max(0, static_cast<int>(seconds));
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%d:%02d", total / 60, total % 60);
    return std::string(buf);
}

Color DapScreen::ArtPlaceholder(const std::string& seed)
{
    uint64_t h = 1469598103934665603ULL;
    for (const char c : seed)
    {
        h ^= static_cast<unsigned char>(c);
        h *= 1099511628211ULL;
    }
    const int hue = static_cast<int>(h % 360);
    const Color palette[] = {
        Color{124, 58, 237, 255},   // violet
        Color{34, 211, 238, 255},   // cyan
        Color{244, 63, 94, 255},    // rose
        Color{52, 211, 153, 255},   // emerald
        Color{250, 204, 21, 255},   // amber
        Color{59, 130, 246, 255},   // blue
        Color{236, 72, 153, 255},   // pink
    };
    return palette[hue % (sizeof(palette) / sizeof(palette[0]))];
}
} // namespace flachead::dap
