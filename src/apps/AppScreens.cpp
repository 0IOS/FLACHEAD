#include "AppScreens.hpp"

#include "../math/Color.hpp"
#include "../math/Rect.hpp"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sstream>
#include <vector>

namespace flachead::apps
{
namespace
{
constexpr float kStatusH = 38.0f;
constexpr float kHeaderH = 42.0f;

const char* const kMonthNames[] = {"January", "February", "March", "April", "May", "June",
                                   "July", "August", "September", "October", "November", "December"};

void FormatTime(char* buffer, size_t bufferSize, float seconds)
{
    const int totalSec = std::max(0, static_cast<int>(seconds));
    const int minutes  = std::min(totalSec / 60, 9999);
    std::snprintf(buffer, bufferSize, "%02d:%02d", minutes, totalSec % 60);
}
} // namespace

// ===========================================================================
// AppScreen Base Methods
// ===========================================================================
void AppScreen::SetBackHandler(std::function<void()> handler)
{
    m_OnBack = std::move(handler);
}

void AppScreen::DrawStatusBar(flachead::ui::Canvas& canvas, int width) const
{
    // Status bar strip
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), kStatusH}, Color{8, 10, 15, 255});
    canvas.DrawLine(0.0f, kStatusH, static_cast<float>(width), kStatusH, Color{30, 36, 51, 255});

    // Wordmark
    canvas.DrawText(Rect{16.0f, 8.0f, 120.0f, 20.0f}, "FLACHEAD OS", Color{248, 250, 252, 255}, 16.0f);

    // Current category / app badge
    canvas.FillRoundedRect(Rect{140.0f, 8.0f, 110.0f, 20.0f}, 4.0f, Color{24, 30, 44, 255});
    canvas.DrawText(Rect{148.0f, 10.0f, 94.0f, 16.0f}, m_Title, Color{124, 58, 237, 255}, 13.0f);

    // Time
    std::time_t t = std::time(nullptr);
    std::tm* tm_info = std::localtime(&t);
    char timeBuf[16];
    std::strftime(timeBuf, sizeof(timeBuf), "%H:%M", tm_info);

    const float rightX = static_cast<float>(width);
    canvas.DrawText(Rect{rightX - 160.0f, 9.0f, 50.0f, 18.0f}, timeBuf, Color{203, 213, 225, 255}, 14.0f);

    // Audio status icon
    canvas.DrawText(Rect{rightX - 104.0f, 9.0f, 18.0f, 18.0f}, "♫", Color{34, 211, 238, 255}, 14.0f);

    // Battery pill
    const float bx = rightX - 74.0f;
    const float by = 9.0f;
    canvas.FillRoundedRect(Rect{bx, by, 58.0f, 18.0f}, 4.0f, Color{18, 24, 36, 255});
    canvas.FillRoundedRect(Rect{bx + 2.0f, by + 2.0f, 44.0f, 14.0f}, 3.0f, Color{34, 211, 238, 255});
    canvas.DrawText(Rect{bx + 6.0f, by + 1.0f, 50.0f, 16.0f}, "82%", Color{8, 10, 15, 255}, 12.0f);
}

void AppScreen::DrawHeader(flachead::ui::Canvas& canvas, int width, const std::string& title,
                           const std::string& subtitle) const
{
    const float y = kStatusH;
    canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), kHeaderH}, Color{12, 15, 23, 255});
    canvas.DrawLine(0.0f, y + kHeaderH, static_cast<float>(width), y + kHeaderH, Color{30, 36, 51, 255});

    canvas.DrawText(Rect{20.0f, y + 8.0f, 260.0f, 24.0f}, title, Color::White, 20.0f);

    if (!subtitle.empty())
    {
        canvas.DrawText(Rect{280.0f, y + 12.0f, 250.0f, 18.0f}, subtitle, Color{148, 163, 184, 255}, 13.0f);
    }

    // Back prompt on right
    canvas.DrawText(Rect{static_cast<float>(width) - 130.0f, y + 11.0f, 110.0f, 18.0f}, "[ESC] Launcher", Color{124, 58, 237, 255}, 13.0f);
}

void AppScreen::DrawFooterHints(flachead::ui::Canvas& canvas, int width, int height,
                                const std::string& hints) const
{
    const float fy = static_cast<float>(height) - 26.0f;
    canvas.FillRect(Rect{0.0f, fy, static_cast<float>(width), 26.0f}, Color{8, 10, 15, 240});
    canvas.DrawLine(0.0f, fy, static_cast<float>(width), fy, Color{30, 36, 51, 255});
    canvas.DrawText(Rect{20.0f, fy + 4.0f, static_cast<float>(width) - 40.0f, 18.0f}, hints, Color{100, 116, 139, 255}, 12.0f);
}

void AppScreen::DrawCard(flachead::ui::Canvas& canvas, const Rect& rect, const std::string& title,
                         const std::string& subtitle, bool selected, const Color& accentColor) const
{
    const Color bg = selected ? Color{28, 35, 51, 255} : Color{18, 22, 32, 255};
    const Color border = selected ? accentColor : Color{30, 36, 51, 255};
    canvas.FillRoundedRect(rect, 8.0f, bg);
    canvas.DrawRoundedRect(rect, 8.0f, border);

    if (selected)
    {
        canvas.DrawRoundedRect(Rect{rect.position.x + 1.0f, rect.position.y + 1.0f, rect.size.x - 2.0f, rect.size.y - 2.0f}, 7.0f, Color{accentColor.r, accentColor.g, accentColor.b, 80});
    }

    canvas.DrawText(Rect{rect.position.x + 14.0f, rect.position.y + 10.0f, rect.size.x - 28.0f, 20.0f}, title, selected ? Color::White : Color{226, 232, 240, 255}, 16.0f);
    canvas.DrawText(Rect{rect.position.x + 14.0f, rect.position.y + 30.0f, rect.size.x - 28.0f, 16.0f}, subtitle, Color{148, 163, 184, 255}, 13.0f);
}

void AppScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)}, Color{8, 10, 15, 255});
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, m_Title);
    DrawFooterHints(canvas, width, height);
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

// ===========================================================================
// MusicScreen Implementation
// ===========================================================================
MusicScreen::MusicScreen()
    : AppScreen("Music", "Audio")
{
}

void MusicScreen::OnEnter()
{
    m_EqualizerBars.fill(0.4f);
}

bool MusicScreen::NeedsRender() const
{
    return m_Playing;
}

void MusicScreen::OnUpdate(float deltaSeconds)
{
    m_PulseAnim += deltaSeconds;

    if (m_Playing)
    {
        m_Spin += deltaSeconds * 45.0f;
        if (m_Spin >= 360.0f) m_Spin -= 360.0f;

        m_ElapsedSeconds += deltaSeconds;
        const auto& track = m_Tracks[m_SelectedTrack];
        m_Progress = std::clamp(m_ElapsedSeconds / track.durationSec, 0.0f, 1.0f);

        if (m_ElapsedSeconds >= track.durationSec)
        {
            m_ElapsedSeconds = 0.0f;
            m_Progress = 0.0f;
            if (m_Repeat)
            {
                m_SelectedTrack = (m_SelectedTrack + 1) % m_Tracks.size();
            }
            else
            {
                m_Playing = false;
            }
        }

        // Animate spectrum visualizer bars
        for (size_t i = 0; i < m_EqualizerBars.size(); ++i)
        {
            float target = 0.15f + 0.85f * std::abs(std::sin(m_PulseAnim * 4.0f + static_cast<float>(i) * 0.65f));
            m_EqualizerBars[i] += (target - m_EqualizerBars[i]) * deltaSeconds * 12.0f;
        }
    }
    else
    {
        for (size_t i = 0; i < m_EqualizerBars.size(); ++i)
        {
            m_EqualizerBars[i] += (0.05f - m_EqualizerBars[i]) * deltaSeconds * 6.0f;
        }
    }
}

void MusicScreen::DrawVinylArtwork(flachead::ui::Canvas& canvas, const Rect& rect, float spin, bool playing, const Track& track) const
{
    canvas.FillRoundedRect(rect, 12.0f, track.artColor);
    canvas.DrawRoundedRect(rect, 12.0f, track.artAccent);

    // Inner vinyl circle
    const float cx = rect.position.x + rect.size.x * 0.5f;
    const float cy = rect.position.y + rect.size.y * 0.5f;
    const float r  = rect.size.x * 0.42f;

    canvas.FillCircle(cx, cy, r, Color{15, 18, 24, 255});
    canvas.DrawCircle(cx, cy, r, Color{40, 48, 64, 255});
    canvas.DrawCircle(cx, cy, r * 0.75f, Color{25, 30, 40, 255});
    canvas.DrawCircle(cx, cy, r * 0.50f, Color{35, 42, 56, 255});

    // Center spindle label
    canvas.FillCircle(cx, cy, r * 0.30f, track.artAccent);
    canvas.FillCircle(cx, cy, 5.0f, Color{8, 10, 15, 255});

    // Rotating tone-arm line indicator when playing
    if (playing)
    {
        const float rad = spin * 3.14159f / 180.0f;
        const float tx = cx + std::cos(rad) * (r * 0.28f);
        const float ty = cy + std::sin(rad) * (r * 0.28f);
        canvas.DrawLine(cx, cy, tx, ty, Color::White);
    }
}

void MusicScreen::DrawVisualizer(flachead::ui::Canvas& canvas, const Rect& rect, bool playing) const
{
    canvas.FillRoundedRect(rect, 6.0f, Color{14, 18, 26, 255});
    canvas.DrawRoundedRect(rect, 6.0f, Color{30, 38, 54, 255});

    const size_t numBars = m_EqualizerBars.size();
    const float barGap = 3.0f;
    const float totalGap = (numBars + 1) * barGap;
    const float barW = (rect.size.x - totalGap) / static_cast<float>(numBars);
    const float maxH = rect.size.y - 12.0f;

    for (size_t i = 0; i < numBars; ++i)
    {
        const float bh = m_EqualizerBars[i] * maxH;
        const float bx = rect.position.x + barGap + i * (barW + barGap);
        const float by = rect.position.y + rect.size.y - 6.0f - bh;

        const Color color = playing ? Color{34, 211, 238, 255} : Color{71, 85, 105, 255};
        canvas.FillRect(Rect{bx, by, barW, bh}, color);
    }
}

void MusicScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    const Track& track = m_Tracks[m_SelectedTrack];

    const int labelSec = static_cast<int>(m_ElapsedSeconds);
    if (labelSec != m_LabelSecond)
    {
        FormatTime(m_ElapsedLabel, sizeof(m_ElapsedLabel), static_cast<float>(labelSec));
        const int remSec    = std::max(0, static_cast<int>(track.durationSec) - labelSec);
        const int remMinute = std::min(remSec / 60, 9999);
        std::snprintf(m_RemainLabel, sizeof(m_RemainLabel), "-%02d:%02d", remMinute, remSec % 60);
        m_LabelSecond = labelSec;
    }

    // Left Album Art Panel
    const Rect artRect{24.0f, 92.0f, 170.0f, 170.0f};
    DrawVinylArtwork(canvas, artRect, m_Spin, m_Playing, track);

    // Track details on right
    const float infoX = 214.0f;
    canvas.DrawText(Rect{infoX, 92.0f, width - infoX - 24.0f, 28.0f}, track.title, Color::White, 24.0f);
    canvas.DrawText(Rect{infoX, 122.0f, width - infoX - 24.0f, 20.0f}, track.artist, track.artAccent, 17.0f);
    canvas.DrawText(Rect{infoX, 144.0f, width - infoX - 24.0f, 18.0f}, track.album, Color{148, 163, 184, 255}, 14.0f);

    // Codec badge pill
    const Rect codecRect{infoX, 168.0f, 170.0f, 22.0f};
    canvas.FillRoundedRect(codecRect, 4.0f, Color{20, 30, 48, 255});
    canvas.DrawRoundedRect(codecRect, 4.0f, Color{34, 211, 238, 255});
    canvas.DrawText(Rect{infoX + 8.0f, 170.0f, 154.0f, 18.0f}, track.codec, Color{34, 211, 238, 255}, 12.0f);

    // Equalizer visualizer bars
    const Rect eqRect{infoX, 198.0f, width - infoX - 24.0f, 64.0f};
    DrawVisualizer(canvas, eqRect, m_Playing);

    // Progress Bar section
    const float progY = 276.0f;
    const Rect progTrack{24.0f, progY, width - 48.0f, 10.0f};
    canvas.FillRoundedRect(progTrack, 5.0f, Color{24, 30, 44, 255});

    const float fillW = progTrack.size.x * m_Progress;
    if (fillW > 0.0f)
    {
        canvas.FillRoundedRect(Rect{progTrack.position.x, progTrack.position.y, fillW, progTrack.size.y}, 5.0f, Color{124, 58, 237, 255});
    }

    // Handle thumb
    canvas.FillCircle(progTrack.position.x + fillW, progTrack.position.y + 5.0f, 7.0f, Color::White);

    // Time indicators
    canvas.DrawText(Rect{24.0f, progY + 14.0f, 80.0f, 18.0f}, m_ElapsedLabel, Color::White, 14.0f);
    canvas.DrawText(Rect{width - 104.0f, progY + 14.0f, 80.0f, 18.0f}, m_RemainLabel, Color{148, 163, 184, 255}, 14.0f);

    // Playback Controls Panel
    const Rect ctrlRect{24.0f, 318.0f, width - 48.0f, 54.0f};
    canvas.FillRoundedRect(ctrlRect, 8.0f, Color{14, 18, 26, 255});
    canvas.DrawRoundedRect(ctrlRect, 8.0f, Color{30, 38, 54, 255});

    const float midX = ctrlRect.position.x + ctrlRect.size.x * 0.5f;

    // Previous button
    canvas.DrawText(Rect{midX - 90.0f, ctrlRect.position.y + 14.0f, 30.0f, 24.0f}, "⏮", Color::White, 20.0f);
    // Rewind 10s
    canvas.DrawText(Rect{midX - 45.0f, ctrlRect.position.y + 16.0f, 30.0f, 20.0f}, "-10s", Color{148, 163, 184, 255}, 13.0f);

    // Play/Pause central button
    const Rect playBtn{midX - 20.0f, ctrlRect.position.y + 7.0f, 40.0f, 40.0f};
    canvas.FillRoundedRect(playBtn, 20.0f, Color{124, 58, 237, 255});
    canvas.DrawText(Rect{playBtn.position.x + (m_Playing ? 12.0f : 14.0f), playBtn.position.y + 10.0f, 20.0f, 20.0f}, m_Playing ? "▮▮" : "▶", Color::White, 16.0f);

    // Forward 10s
    canvas.DrawText(Rect{midX + 25.0f, ctrlRect.position.y + 16.0f, 30.0f, 20.0f}, "+10s", Color{148, 163, 184, 255}, 13.0f);
    // Next button
    canvas.DrawText(Rect{midX + 70.0f, ctrlRect.position.y + 14.0f, 30.0f, 24.0f}, "⏭", Color::White, 20.0f);

    // Shuffle & Repeat indicators
    canvas.DrawText(Rect{ctrlRect.position.x + 20.0f, ctrlRect.position.y + 18.0f, 40.0f, 20.0f}, "🔀", m_Shuffle ? Color{34, 211, 238, 255} : Color{71, 85, 105, 255}, 14.0f);
    canvas.DrawText(Rect{ctrlRect.position.x + ctrlRect.size.x - 50.0f, ctrlRect.position.y + 18.0f, 40.0f, 20.0f}, "🔁", m_Repeat ? Color{34, 211, 238, 255} : Color{71, 85, 105, 255}, 14.0f);

    // Playlist Selector strip
    const float cardW = (width - 48.0f - 24.0f) / 3.0f;
    for (size_t i = 0; i < 3; ++i)
    {
        const size_t idx = (m_SelectedTrack + i) % m_Tracks.size();
        const Rect tCard{24.0f + i * (cardW + 12.0f), 384.0f, cardW, 46.0f};
        const bool active = (idx == static_cast<size_t>(m_SelectedTrack));
        DrawCard(canvas, tCard, m_Tracks[idx].title, m_Tracks[idx].artist, active, m_Tracks[idx].artAccent);
    }

    DrawFooterHints(canvas, width, height, "[LEFT/RIGHT] Track  [SPACE] Play/Pause  [UP/DOWN] Volume  [S] Shuffle  [ESC] Back");
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
                m_SelectedTrack = (m_SelectedTrack + static_cast<int>(m_Tracks.size()) - 1) % m_Tracks.size();
                m_ElapsedSeconds = 0.0f;
                m_Progress = 0.0f;
                return true;
            case SDLK_RIGHT:
                m_SelectedTrack = (m_SelectedTrack + 1) % m_Tracks.size();
                m_ElapsedSeconds = 0.0f;
                m_Progress = 0.0f;
                return true;
            case SDLK_SPACE:
                m_Playing = !m_Playing;
                return true;
            case SDLK_UP:
                m_Volume = std::min(1.0f, m_Volume + 0.05f);
                return true;
            case SDLK_DOWN:
                m_Volume = std::max(0.0f, m_Volume - 0.05f);
                return true;
            case SDLK_S:
                m_Shuffle = !m_Shuffle;
                return true;
            case SDLK_R:
                m_Repeat = !m_Repeat;
                return true;
            default:
                break;
        }
    }
    return false;
}

// ===========================================================================
// GalleryScreen Implementation
// ===========================================================================
GalleryScreen::GalleryScreen()
    : AppScreen("Gallery", "Media")
{
}

void GalleryScreen::OnUpdate(float deltaSeconds)
{
    if (m_Fullscreen)
    {
        m_FullscreenAnim = std::min(1.0f, m_FullscreenAnim + deltaSeconds * 6.0f);
    }
    else
    {
        m_FullscreenAnim = std::max(0.0f, m_FullscreenAnim - deltaSeconds * 6.0f);
    }
}

bool GalleryScreen::NeedsRender() const
{
    return m_FullscreenAnim > 0.0f && m_FullscreenAnim < 1.0f;
}

void GalleryScreen::DrawThumbnailPattern(flachead::ui::Canvas& canvas, const Rect& rect, const Photo& photo) const
{
    canvas.FillRoundedRect(rect, 8.0f, photo.bg);

    const float cx = rect.position.x + rect.size.x * 0.5f;
    const float cy = rect.position.y + rect.size.y * 0.4f;

    switch (photo.patternType)
    {
        case 0: // Sunset
            canvas.FillCircle(cx, cy + 10.0f, 24.0f, photo.fg);
            canvas.FillRect(Rect{rect.position.x, cy + 10.0f, rect.size.x, rect.size.y - (cy + 10.0f - rect.position.y)}, Color{15, 20, 30, 200});
            break;
        case 1: // Snow
            canvas.DrawLine(rect.position.x + 10.0f, rect.position.y + rect.size.y - 10.0f, cx, rect.position.y + 15.0f, photo.fg);
            canvas.DrawLine(cx, rect.position.y + 15.0f, rect.position.x + rect.size.x - 10.0f, rect.position.y + rect.size.y - 10.0f, photo.fg);
            break;
        case 2: // Cyber Grid
            for (int i = -2; i <= 2; ++i)
            {
                canvas.DrawLine(cx + i * 20.0f, rect.position.y + 10.0f, cx + i * 35.0f, rect.position.y + rect.size.y - 10.0f, photo.fg);
            }
            break;
        case 3: // Waveform
            for (int i = 0; i < 7; ++i)
            {
                float h = 10.0f + 25.0f * std::sin(i * 0.8f);
                canvas.FillRect(Rect{rect.position.x + 20.0f + i * 18.0f, cy - h * 0.5f, 10.0f, h}, photo.fg);
            }
            break;
        case 4: // Architectural
            canvas.DrawRoundedRect(Rect{cx - 30.0f, cy - 25.0f, 60.0f, 50.0f}, 4.0f, photo.fg);
            canvas.DrawRoundedRect(Rect{cx - 18.0f, cy - 15.0f, 36.0f, 30.0f}, 2.0f, photo.fg);
            break;
        case 5: // Eclipse
            canvas.FillCircle(cx, cy, 28.0f, photo.fg);
            canvas.FillCircle(cx - 6.0f, cy - 4.0f, 26.0f, photo.bg);
            break;
        default:
            break;
    }
}

void GalleryScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    const int cols = 3;
    const float spacing = 14.0f;
    const float gridX = 24.0f;
    const float gridY = 92.0f;
    const float cardW = (width - 48.0f - (cols - 1) * spacing) / static_cast<float>(cols);
    const float cardH = 150.0f;

    for (size_t i = 0; i < m_Photos.size(); ++i)
    {
        const int row = static_cast<int>(i) / cols;
        const int col = static_cast<int>(i) % cols;
        const Rect rect{gridX + col * (cardW + spacing), gridY + row * (cardH + spacing), cardW, cardH};

        const bool sel = (static_cast<int>(i) == m_SelectedIndex);

        // Thumbnail artwork
        const Rect thumbRect{rect.position.x, rect.position.y, rect.size.x, 105.0f};
        DrawThumbnailPattern(canvas, thumbRect, m_Photos[i]);

        // Card info footer
        const Rect labelRect{rect.position.x, rect.position.y + 105.0f, rect.size.x, 45.0f};
        canvas.FillRoundedRect(labelRect, 4.0f, sel ? Color{28, 35, 51, 255} : Color{18, 22, 32, 255});
        canvas.DrawRoundedRect(rect, 8.0f, sel ? Color{124, 58, 237, 255} : Color{30, 36, 51, 255});

        canvas.DrawText(Rect{rect.position.x + 10.0f, rect.position.y + 110.0f, rect.size.x - 20.0f, 18.0f}, m_Photos[i].title, sel ? Color::White : Color{226, 232, 240, 255}, 14.0f);
        canvas.DrawText(Rect{rect.position.x + 10.0f, rect.position.y + 128.0f, rect.size.x - 20.0f, 16.0f}, m_Photos[i].meta, Color{148, 163, 184, 255}, 12.0f);
    }

    // Fullscreen Image Overlay
    if (m_Fullscreen || m_FullscreenAnim > 0.0f)
    {
        const Photo& photo = m_Photos[m_SelectedIndex];
        canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)}, Color{0, 0, 0, static_cast<uint8_t>(235 * m_FullscreenAnim)});

        const float fw = 560.0f * m_FullscreenAnim;
        const float fh = 320.0f * m_FullscreenAnim;
        const Rect fRect{(width - fw) * 0.5f, 70.0f, fw, fh};

        DrawThumbnailPattern(canvas, fRect, photo);
        canvas.DrawRoundedRect(fRect, 10.0f, Color::White);

        // Details overlay bar below image
        const Rect detailsBar{fRect.position.x, fRect.position.y + fRect.size.y + 12.0f, fRect.size.x, 50.0f};
        canvas.FillRoundedRect(detailsBar, 6.0f, Color{18, 24, 36, 240});
        canvas.DrawText(Rect{detailsBar.position.x + 14.0f, detailsBar.position.y + 8.0f, 300.0f, 20.0f}, photo.title, Color::White, 16.0f);

        char metaBuf[96];
        std::snprintf(metaBuf, sizeof(metaBuf), "%s · %s", photo.meta.c_str(), photo.camera.c_str());
        canvas.DrawText(Rect{detailsBar.position.x + 14.0f, detailsBar.position.y + 28.0f, 300.0f, 16.0f}, metaBuf, Color{148, 163, 184, 255}, 13.0f);
    }

    DrawFooterHints(canvas, width, height, m_Fullscreen ? "[ESC] Close Fullscreen  [ARROWS] Next Image" : "[ARROWS] Navigate  [ENTER] Fullscreen Preview  [ESC] Back");
}

bool GalleryScreen::HandleEvent(const SDL_Event& event)
{
    if (m_Fullscreen)
    {
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            if (event.key.key == SDLK_ESCAPE || event.key.key == SDLK_RETURN)
            {
                m_Fullscreen = false;
                return true;
            }
            if (event.key.key == SDLK_LEFT)
            {
                m_SelectedIndex = (m_SelectedIndex + static_cast<int>(m_Photos.size()) - 1) % m_Photos.size();
                return true;
            }
            if (event.key.key == SDLK_RIGHT)
            {
                m_SelectedIndex = (m_SelectedIndex + 1) % m_Photos.size();
                return true;
            }
        }
        return true;
    }

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
                m_SelectedIndex = std::min(static_cast<int>(m_Photos.size()) - 1, m_SelectedIndex + 1);
                return true;
            case SDLK_UP:
                m_SelectedIndex = std::max(0, m_SelectedIndex - 3);
                return true;
            case SDLK_DOWN:
                m_SelectedIndex = std::min(static_cast<int>(m_Photos.size()) - 1, m_SelectedIndex + 3);
                return true;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                m_Fullscreen = true;
                return true;
            default:
                break;
        }
    }

    return false;
}

// ===========================================================================
// VideoScreen Implementation
// ===========================================================================
VideoScreen::VideoScreen()
    : AppScreen("Video", "Media")
{
}

void VideoScreen::OnUpdate(float deltaSeconds)
{
    m_AnimTimer += deltaSeconds;

    if (m_Playing)
    {
        m_ElapsedSeconds += deltaSeconds;
        const auto& vid = m_Videos[m_SelectedVideo];
        m_Progress = std::clamp(m_ElapsedSeconds / vid.durationSec, 0.0f, 1.0f);

        if (m_ElapsedSeconds >= vid.durationSec)
        {
            m_ElapsedSeconds = 0.0f;
            m_Progress = 0.0f;
        }
    }
}

bool VideoScreen::NeedsRender() const
{
    return m_Playing;
}

void VideoScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    const auto& vid = m_Videos[m_SelectedVideo];

    // Video Screen Viewport
    const Rect viewRect{24.0f, 92.0f, width - 48.0f, 210.0f};
    canvas.FillRoundedRect(viewRect, 10.0f, vid.color);
    canvas.DrawRoundedRect(viewRect, 10.0f, Color::White);

    // Animated grid background simulating video playback
    const float vcx = viewRect.position.x + viewRect.size.x * 0.5f;
    const float vcy = viewRect.position.y + viewRect.size.y * 0.5f;

    const float offset = m_Playing ? std::fmod(m_AnimTimer * 60.0f, 30.0f) : 0.0f;

    for (int i = -6; i <= 6; ++i)
    {
        canvas.DrawLine(vcx + i * 40.0f + offset, viewRect.position.y, vcx + i * 20.0f, viewRect.position.y + viewRect.size.y, Color{255, 255, 255, 40});
    }

    // Video overlay title watermark
    canvas.DrawText(Rect{viewRect.position.x + 16.0f, viewRect.position.y + 16.0f, 300.0f, 22.0f}, vid.title, Color::White, 18.0f);
    canvas.DrawText(Rect{viewRect.position.x + 16.0f, viewRect.position.y + 40.0f, 300.0f, 18.0f}, vid.codec + " · " + vid.resolution, Color{34, 211, 238, 255}, 13.0f);

    // Paused overlay indicator
    if (!m_Playing)
    {
        canvas.FillRoundedRect(Rect{vcx - 60.0f, vcy - 20.0f, 120.0f, 40.0f}, 6.0f, Color{8, 10, 15, 220});
        canvas.DrawText(Rect{vcx - 45.0f, vcy - 10.0f, 90.0f, 20.0f}, "PAUSED", Color::White, 16.0f);
    }

    // Timeline bar
    const float timeY = 312.0f;
    const Rect timeTrack{24.0f, timeY, width - 48.0f, 8.0f};
    canvas.FillRoundedRect(timeTrack, 4.0f, Color{24, 30, 44, 255});
    canvas.FillRoundedRect(Rect{timeTrack.position.x, timeTrack.position.y, timeTrack.size.x * m_Progress, timeTrack.size.y}, 4.0f, Color{34, 211, 238, 255});

    // Controls strip
    const Rect ctrlRect{24.0f, 330.0f, width - 48.0f, 48.0f};
    canvas.FillRoundedRect(ctrlRect, 6.0f, Color{14, 18, 26, 255});
    canvas.DrawRoundedRect(ctrlRect, 6.0f, Color{30, 38, 54, 255});

    canvas.DrawText(Rect{ctrlRect.position.x + 20.0f, ctrlRect.position.y + 14.0f, 30.0f, 20.0f}, m_Playing ? "▮▮" : "▶", Color::White, 16.0f);

    const int labelSec = static_cast<int>(m_ElapsedSeconds);
    if (labelSec != m_LabelSecond)
    {
        char elapsed[8];
        FormatTime(elapsed, sizeof(elapsed), static_cast<float>(labelSec));
        std::snprintf(m_TimeLabel, sizeof(m_TimeLabel), "%s / %s", elapsed, vid.durationStr.c_str());
        m_LabelSecond = labelSec;
    }
    canvas.DrawText(Rect{ctrlRect.position.x + 60.0f, ctrlRect.position.y + 15.0f, 200.0f, 18.0f}, m_TimeLabel, Color::White, 14.0f);

    // Video selection cards strip
    const float cardW = (width - 48.0f - 24.0f) / 3.0f;
    for (size_t i = 0; i < m_Videos.size(); ++i)
    {
        const Rect vCard{24.0f + i * (cardW + 12.0f), 390.0f, cardW, 42.0f};
        const bool active = (static_cast<int>(i) == m_SelectedVideo);
        DrawCard(canvas, vCard, m_Videos[i].title, m_Videos[i].durationStr, active, Color{34, 211, 238, 255});
    }

    DrawFooterHints(canvas, width, height, "[LEFT/RIGHT] Select Video  [SPACE] Play/Pause  [ESC] Back");
}

bool VideoScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_SPACE:
                m_Playing = !m_Playing;
                return true;
            case SDLK_LEFT:
                m_SelectedVideo = (m_SelectedVideo + static_cast<int>(m_Videos.size()) - 1) % m_Videos.size();
                m_ElapsedSeconds = 0.0f;
                m_Progress = 0.0f;
                return true;
            case SDLK_RIGHT:
                m_SelectedVideo = (m_SelectedVideo + 1) % m_Videos.size();
                m_ElapsedSeconds = 0.0f;
                m_Progress = 0.0f;
                return true;
            default:
                break;
        }
    }
    return false;
}

// ===========================================================================
// CalculatorScreen Implementation
// ===========================================================================
CalculatorScreen::CalculatorScreen()
    : AppScreen("Calculator", "Tools")
{
}

void CalculatorScreen::InputChar(char ch)
{
    if (m_NewEntry)
    {
        m_Display = std::string(1, ch);
        m_NewEntry = false;
    }
    else
    {
        if (m_Display == "0" && ch != '.')
        {
            m_Display = std::string(1, ch);
        }
        else
        {
            if (ch == '.' && m_Display.find('.') != std::string::npos)
                return;
            m_Display += ch;
        }
    }
}

void CalculatorScreen::InputOp(char op)
{
    try
    {
        m_Accumulator = std::stod(m_Display);
    }
    catch (...)
    {
        m_Accumulator = 0.0;
    }
    m_CurrentOp = op;
    m_Expression = m_Display + " " + std::string(1, op);
    m_NewEntry = true;
}

void CalculatorScreen::CalculateResult()
{
    if (m_CurrentOp == '\0') return;

    double val = 0.0;
    try { val = std::stod(m_Display); } catch (...) { val = 0.0; }

    double result = 0.0;
    if (m_CurrentOp == '+') result = m_Accumulator + val;
    else if (m_CurrentOp == '-') result = m_Accumulator - val;
    else if (m_CurrentOp == '*') result = m_Accumulator * val;
    else if (m_CurrentOp == '/') result = (val != 0.0) ? m_Accumulator / val : 0.0;

    std::ostringstream ss;
    if (std::abs(result - std::round(result)) < 1e-7)
    {
        ss << static_cast<long long>(std::round(result));
    }
    else
    {
        ss << result;
    }

    m_Expression += " " + m_Display + " =";
    m_Display = ss.str();
    m_CurrentOp = '\0';
    m_NewEntry = true;
}

void CalculatorScreen::ClearAll()
{
    m_Display = "0";
    m_Expression.clear();
    m_Accumulator = 0.0;
    m_CurrentOp = '\0';
    m_NewEntry = true;
}

void CalculatorScreen::Backspace()
{
    if (!m_Display.empty() && m_Display != "0")
    {
        m_Display.pop_back();
        if (m_Display.empty()) m_Display = "0";
    }
}

void CalculatorScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    // Digital Display Box
    const Rect dispRect{24.0f, 90.0f, width - 48.0f, 68.0f};
    canvas.FillRoundedRect(dispRect, 8.0f, Color{14, 18, 26, 255});
    canvas.DrawRoundedRect(dispRect, 8.0f, Color{30, 38, 54, 255});

    // Expression line
    canvas.DrawText(Rect{dispRect.position.x + 16.0f, dispRect.position.y + 8.0f, dispRect.size.x - 32.0f, 18.0f}, m_Expression, Color{148, 163, 184, 255}, 13.0f);
    // Main Display text
    canvas.DrawText(Rect{dispRect.position.x + 16.0f, dispRect.position.y + 28.0f, dispRect.size.x - 32.0f, 32.0f}, m_Display, Color::White, 28.0f);

    // Keypad Grid
    const float pad = 8.0f;
    const float startY = 170.0f;
    const float gridW = width - 48.0f;
    const float btnW = (gridW - pad * 3.0f) / 4.0f;
    const float btnH = 46.0f;

    for (int r = 0; r < static_cast<int>(m_Keypad.size()); ++r)
    {
        for (int c = 0; c < static_cast<int>(m_Keypad[r].size()); ++c)
        {
            const Rect bRect{24.0f + c * (btnW + pad), startY + r * (btnH + pad), btnW, btnH};
            const bool focused = (r == m_FocusedRow && c == m_FocusedCol);
            const std::string& label = m_Keypad[r][c];

            const bool isOp = (label == "+" || label == "-" || label == "*" || label == "/" || label == "=");
            const Color bg = focused ? Color{124, 58, 237, 255} : (isOp ? Color{24, 32, 48, 255} : Color{18, 22, 32, 255});
            const Color border = focused ? Color::White : Color{30, 36, 51, 255};

            canvas.FillRoundedRect(bRect, 6.0f, bg);
            canvas.DrawRoundedRect(bRect, 6.0f, border);

            canvas.DrawText(Rect{bRect.position.x + bRect.size.x * 0.40f, bRect.position.y + 12.0f, 30.0f, 24.0f}, label, focused ? Color::White : (isOp ? Color{34, 211, 238, 255} : Color::White), 20.0f);
        }
    }

    DrawFooterHints(canvas, width, height, "[ARROWS] Move  [ENTER] Press Key  [NUMKEYS] Direct Typing  [ESC] Back");
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

        // Direct digit keys
        if (key >= SDLK_0 && key <= SDLK_9)
        {
            InputChar(static_cast<char>('0' + (key - SDLK_0)));
            return true;
        }

        switch (key)
        {
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
                InputOp('+'); return true;
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                InputOp('-'); return true;
            case SDLK_ASTERISK:
            case SDLK_KP_MULTIPLY:
                InputOp('*'); return true;
            case SDLK_SLASH:
            case SDLK_KP_DIVIDE:
                InputOp('/'); return true;
            case SDLK_PERIOD:
            case SDLK_KP_PERIOD:
                InputChar('.'); return true;
            case SDLK_EQUALS:
            case SDLK_KP_ENTER:
            case SDLK_RETURN:
            {
                const std::string& label = m_Keypad[m_FocusedRow][m_FocusedCol];
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
                {
                    if (label == "C") ClearAll();
                    else if (label == "⌫") Backspace();
                    else if (label == "=") CalculateResult();
                    else if (label == "+" || label == "-" || label == "*" || label == "/") InputOp(label[0]);
                    else if (label.size() == 1) InputChar(label[0]);
                }
                else
                {
                    CalculateResult();
                }
                return true;
            }
            case SDLK_BACKSPACE:
                Backspace(); return true;
            case SDLK_C:
                ClearAll(); return true;
            case SDLK_LEFT:
                m_FocusedCol = (m_FocusedCol + 3) % 4; return true;
            case SDLK_RIGHT:
                m_FocusedCol = (m_FocusedCol + 1) % 4; return true;
            case SDLK_UP:
                m_FocusedRow = (m_FocusedRow + 4) % 5; return true;
            case SDLK_DOWN:
                m_FocusedRow = (m_FocusedRow + 1) % 5; return true;
            default:
                break;
        }
    }

    return false;
}

// ===========================================================================
// CalendarScreen Implementation
// ===========================================================================
CalendarScreen::CalendarScreen()
    : AppScreen("Calendar", "Schedule")
{
}

int CalendarScreen::DaysInMonth(int year, int month)
{
    if (month == 2)
    {
        bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        return leap ? 29 : 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
    return 31;
}

int CalendarScreen::StartWeekday(int year, int month)
{
    // Zeller's congruence adaptation for 1-indexed weekday (0=Mon, 6=Sun)
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = 1;
    std::mktime(&tm);
    int w = tm.tm_wday - 1; // Convert Sun=0 -> Mon=0
    if (w < 0) w += 7;
    return w;
}

void CalendarScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    // Month Selector Header Bar
    const Rect monthHeader{24.0f, 92.0f, 370.0f, 42.0f};
    canvas.FillRoundedRect(monthHeader, 6.0f, Color{14, 18, 26, 255});
    canvas.DrawRoundedRect(monthHeader, 6.0f, Color{30, 38, 54, 255});

    char mTitle[40];
    std::snprintf(mTitle, sizeof(mTitle), "%s %d", kMonthNames[m_Month - 1], m_Year);

    canvas.DrawText(Rect{monthHeader.position.x + 16.0f, monthHeader.position.y + 11.0f, 20.0f, 20.0f}, "◀", Color{124, 58, 237, 255}, 16.0f);
    canvas.DrawText(Rect{monthHeader.position.x + 110.0f, monthHeader.position.y + 10.0f, 180.0f, 22.0f}, mTitle, Color::White, 18.0f);
    canvas.DrawText(Rect{monthHeader.position.x + monthHeader.size.x - 30.0f, monthHeader.position.y + 11.0f, 20.0f, 20.0f}, "▶", Color{124, 58, 237, 255}, 16.0f);

    // Weekday headers
    const char* const days[] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
    const float cellW = 48.0f;
    const float cellH = 34.0f;
    const float gap = 5.0f;

    for (int i = 0; i < 7; ++i)
    {
        const Rect dRect{24.0f + i * (cellW + gap), 142.0f, cellW, 24.0f};
        canvas.DrawText(Rect{dRect.position.x + 14.0f, dRect.position.y + 4.0f, 30.0f, 18.0f}, days[i], Color{148, 163, 184, 255}, 13.0f);
    }

    // Days Grid
    const int startWd = StartWeekday(m_Year, m_Month);
    const int totalDays = DaysInMonth(m_Year, m_Month);

    for (int d = 1; d <= totalDays; ++d)
    {
        const int idx = startWd + (d - 1);
        const int row = idx / 7;
        const int col = idx % 7;

        const Rect cRect{24.0f + col * (cellW + gap), 170.0f + row * (cellH + gap), cellW, cellH};
        const bool selected = (d == m_SelectedDay);
        const bool today = (d == 26 && m_Month == 7);

        const Color bg = selected ? Color{124, 58, 237, 255} : (today ? Color{24, 40, 60, 255} : Color{18, 22, 32, 255});
        const Color border = selected ? Color::White : (today ? Color{34, 211, 238, 255} : Color{30, 36, 51, 255});

        canvas.FillRoundedRect(cRect, 4.0f, bg);
        canvas.DrawRoundedRect(cRect, 4.0f, border);

        char dayBuf[8];
        std::snprintf(dayBuf, sizeof(dayBuf), "%d", d);
        canvas.DrawText(Rect{cRect.position.x + 14.0f, cRect.position.y + 8.0f, 24.0f, 18.0f}, dayBuf, selected ? Color::White : Color{226, 232, 240, 255}, 14.0f);

        // Check if day has event
        for (const auto& ev : m_Events)
        {
            if (ev.day == d)
            {
                canvas.FillCircle(cRect.position.x + cRect.size.x - 8.0f, cRect.position.y + 8.0f, 3.0f, Color{34, 211, 238, 255});
                break;
            }
        }
    }

    // Agenda / Events Panel on right
    const float agendaX = 410.0f;
    const Rect agendaRect{agendaX, 92.0f, width - agendaX - 24.0f, 330.0f};
    canvas.FillRoundedRect(agendaRect, 8.0f, Color{14, 18, 26, 255});
    canvas.DrawRoundedRect(agendaRect, 8.0f, Color{30, 38, 54, 255});

    char agendaBuf[48];
    std::snprintf(agendaBuf, sizeof(agendaBuf), "Agenda • Day %d", m_SelectedDay);
    canvas.DrawText(Rect{agendaRect.position.x + 16.0f, agendaRect.position.y + 14.0f, 200.0f, 22.0f}, agendaBuf, Color::White, 17.0f);
    canvas.DrawLine(agendaRect.position.x + 16.0f, agendaRect.position.y + 42.0f, agendaRect.position.x + agendaRect.size.x - 16.0f, agendaRect.position.y + 42.0f, Color{30, 36, 51, 255});

    float evY = agendaRect.position.y + 54.0f;
    bool foundAny = false;

    for (const auto& ev : m_Events)
    {
        if (ev.day == m_SelectedDay)
        {
            foundAny = true;
            const Rect eCard{agendaRect.position.x + 14.0f, evY, agendaRect.size.x - 28.0f, 54.0f};
            canvas.FillRoundedRect(eCard, 6.0f, Color{22, 28, 42, 255});
            canvas.DrawRoundedRect(eCard, 6.0f, Color{124, 58, 237, 255});

            canvas.DrawText(Rect{eCard.position.x + 12.0f, eCard.position.y + 8.0f, 200.0f, 20.0f}, ev.title, Color::White, 15.0f);
            canvas.DrawText(Rect{eCard.position.x + 12.0f, eCard.position.y + 28.0f, 200.0f, 16.0f}, ev.time + " · " + ev.category, Color{34, 211, 238, 255}, 12.0f);

            evY += 64.0f;
        }
    }

    if (!foundAny)
    {
        canvas.DrawText(Rect{agendaRect.position.x + 16.0f, evY + 20.0f, 200.0f, 20.0f}, "No events scheduled", Color{100, 116, 139, 255}, 14.0f);
    }

    DrawFooterHints(canvas, width, height, "[LEFT/RIGHT] Day  [UP/DOWN] Month  [ESC] Back");
}

bool CalendarScreen::HandleEvent(const SDL_Event& event)
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
                m_SelectedDay = std::max(1, m_SelectedDay - 1);
                return true;
            case SDLK_RIGHT:
                m_SelectedDay = std::min(DaysInMonth(m_Year, m_Month), m_SelectedDay + 1);
                return true;
            case SDLK_UP:
                m_Month = (m_Month == 1) ? 12 : m_Month - 1;
                m_SelectedDay = std::min(m_SelectedDay, DaysInMonth(m_Year, m_Month));
                return true;
            case SDLK_DOWN:
                m_Month = (m_Month == 12) ? 1 : m_Month + 1;
                m_SelectedDay = std::min(m_SelectedDay, DaysInMonth(m_Year, m_Month));
                return true;
            default:
                break;
        }
    }

    return false;
}

// ===========================================================================
// NotesScreen Implementation
// ===========================================================================
NotesScreen::NotesScreen()
    : AppScreen("Notes", "Office")
{
}

void NotesScreen::OnUpdate(float deltaSeconds)
{
    m_CursorBlink += deltaSeconds * 3.0f;
    m_CursorVisible = (static_cast<int>(m_CursorBlink) % 2 == 0);

    if (m_SavedNotificationTimer > 0.0f)
    {
        m_SavedNotificationTimer -= deltaSeconds;
    }
}

bool NotesScreen::NeedsRender() const
{
    return m_CursorVisible;
}

void NotesScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    // Left Notes List Sidebar
    const float sideW = 180.0f;
    const Rect sideRect{24.0f, 92.0f, sideW, 330.0f};
    canvas.FillRoundedRect(sideRect, 8.0f, Color{14, 18, 26, 255});
    canvas.DrawRoundedRect(sideRect, 8.0f, Color{30, 38, 54, 255});

    canvas.DrawText(Rect{sideRect.position.x + 14.0f, sideRect.position.y + 12.0f, 150.0f, 20.0f}, "DOCUMENTS", Color{148, 163, 184, 255}, 13.0f);

    for (size_t i = 0; i < m_Notes.size(); ++i)
    {
        const Rect nCard{sideRect.position.x + 10.0f, sideRect.position.y + 40.0f + i * 50.0f, sideW - 20.0f, 42.0f};
        const bool active = (static_cast<int>(i) == m_ActiveNote);
        DrawCard(canvas, nCard, m_Notes[i].title, m_Notes[i].filename, active, Color{124, 58, 237, 255});
    }

    // Right Text Editor Box
    const float editorX = 218.0f;
    const Rect edRect{editorX, 92.0f, width - editorX - 24.0f, 330.0f};
    canvas.FillRoundedRect(edRect, 8.0f, Color{18, 22, 32, 255});
    canvas.DrawRoundedRect(edRect, 8.0f, Color{30, 38, 54, 255});

    auto& doc = m_Notes[m_ActiveNote];

    // Document header bar inside editor
    canvas.DrawText(Rect{edRect.position.x + 16.0f, edRect.position.y + 12.0f, 200.0f, 20.0f}, doc.filename, Color::White, 16.0f);
    canvas.DrawText(Rect{edRect.position.x + edRect.size.x - 100.0f, edRect.position.y + 14.0f, 80.0f, 18.0f}, m_Saved ? "[Saved]" : "* Modified", m_Saved ? Color{34, 211, 238, 255} : Color{245, 158, 11, 255}, 13.0f);
    canvas.DrawLine(edRect.position.x + 16.0f, edRect.position.y + 38.0f, edRect.position.x + edRect.size.x - 16.0f, edRect.position.y + 38.0f, Color{30, 36, 51, 255});

    // Content multiline display
    const std::string& content = doc.content;
    float lineY = edRect.position.y + 48.0f;
    size_t charAccum = 0;
    size_t start = 0;
    bool cursorDrawn = false;
    const float lineLimit = edRect.position.y + edRect.size.y - 30.0f;

    while (start < content.size() && lineY <= lineLimit)
    {
        size_t end = content.find('\n', start);
        if (end == std::string::npos)
        {
            end = content.size();
        }
        const std::string_view line(content.data() + start, end - start);

        canvas.DrawText(Rect{edRect.position.x + 16.0f, lineY, edRect.size.x - 32.0f, 20.0f}, line, Color{241, 245, 249, 255}, 15.0f);

        // Blinking cursor calculation
        if (!cursorDrawn && doc.cursorPos >= charAccum && doc.cursorPos <= charAccum + line.size())
        {
            size_t relPos = doc.cursorPos - charAccum;
            float curX = edRect.position.x + 16.0f + relPos * 9.0f;
            if (m_CursorVisible)
            {
                canvas.FillRect(Rect{curX, lineY, 2.0f, 18.0f}, Color{124, 58, 237, 255});
            }
            cursorDrawn = true;
        }

        charAccum += line.size() + 1;
        start = end + 1;
        lineY += 24.0f;
    }

    if (!cursorDrawn && m_CursorVisible)
    {
        canvas.FillRect(Rect{edRect.position.x + 16.0f, lineY, 2.0f, 18.0f}, Color{124, 58, 237, 255});
    }

    DrawFooterHints(canvas, width, height, "[TAB] Switch Note  [TYPE] Edit Text  [BACKSPACE] Erase  [ESC] Back");
}

bool NotesScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    auto& doc = m_Notes[m_ActiveNote];

    if (event.type == SDL_EVENT_TEXT_INPUT)
    {
        doc.content.insert(doc.cursorPos, event.text.text);
        doc.cursorPos += std::strlen(event.text.text);
        m_Saved = false;
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_TAB:
                m_ActiveNote = (m_ActiveNote + 1) % m_Notes.size();
                return true;
            case SDLK_BACKSPACE:
                if (doc.cursorPos > 0 && !doc.content.empty())
                {
                    doc.content.erase(doc.cursorPos - 1, 1);
                    doc.cursorPos--;
                    m_Saved = false;
                }
                return true;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                doc.content.insert(doc.cursorPos, "\n");
                doc.cursorPos++;
                m_Saved = false;
                return true;
            case SDLK_LEFT:
                if (doc.cursorPos > 0) doc.cursorPos--;
                return true;
            case SDLK_RIGHT:
                if (doc.cursorPos < doc.content.size()) doc.cursorPos++;
                return true;
            default:
                break;
        }
    }

    return false;
}

// ===========================================================================
// SettingsScreen Implementation
// ===========================================================================
SettingsScreen::SettingsScreen()
    : AppScreen("Settings", "System")
{
}

void SettingsScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    const float startY = 92.0f;
    const float itemH = 48.0f;
    const float gap = 8.0f;

    for (size_t i = 0; i < m_Items.size(); ++i)
    {
        const Rect rect{24.0f, startY + i * (itemH + gap), width - 48.0f, itemH};
        const bool selected = (static_cast<int>(i) == m_SelectedIndex);

        const auto& item = m_Items[i];

        const Color bg = selected ? Color{28, 35, 51, 255} : Color{18, 22, 32, 255};
        const Color border = selected ? Color{124, 58, 237, 255} : Color{30, 36, 51, 255};

        canvas.FillRoundedRect(rect, 6.0f, bg);
        canvas.DrawRoundedRect(rect, 6.0f, border);

        // Name & Description
        canvas.DrawText(Rect{rect.position.x + 16.0f, rect.position.y + 8.0f, 220.0f, 20.0f}, item.name, selected ? Color::White : Color{226, 232, 240, 255}, 15.0f);
        canvas.DrawText(Rect{rect.position.x + 16.0f, rect.position.y + 26.0f, 220.0f, 16.0f}, item.desc, Color{148, 163, 184, 255}, 12.0f);

        // Control Widget Rendering on Right
        const float ctrlX = rect.position.x + rect.size.x - 180.0f;

        if (item.type == ItemType::Toggle)
        {
            const Rect swRect{ctrlX + 110.0f, rect.position.y + 12.0f, 50.0f, 24.0f};
            canvas.FillRoundedRect(swRect, 12.0f, item.toggleVal ? Color{124, 58, 237, 255} : Color{47, 55, 73, 255});
            const float circleX = item.toggleVal ? swRect.position.x + 36.0f : swRect.position.x + 12.0f;
            canvas.FillCircle(circleX, swRect.position.y + 12.0f, 9.0f, Color::White);
        }
        else if (item.type == ItemType::Slider)
        {
            const Rect sTrack{ctrlX + 20.0f, rect.position.y + 20.0f, 140.0f, 8.0f};
            canvas.FillRoundedRect(sTrack, 4.0f, Color{30, 38, 54, 255});
            canvas.FillRoundedRect(Rect{sTrack.position.x, sTrack.position.y, sTrack.size.x * item.sliderVal, sTrack.size.y}, 4.0f, Color{34, 211, 238, 255});
            canvas.FillCircle(sTrack.position.x + sTrack.size.x * item.sliderVal, sTrack.position.y + 4.0f, 7.0f, Color::White);
        }
        else if (item.type == ItemType::Picker)
        {
            const std::string& opt = item.options[item.optionIdx];
            char pickBuf[96];
            std::snprintf(pickBuf, sizeof(pickBuf), "◄ %s ►", opt.c_str());
            canvas.DrawText(Rect{ctrlX + 20.0f, rect.position.y + 14.0f, 140.0f, 20.0f}, pickBuf, Color{34, 211, 238, 255}, 14.0f);
        }
        else if (item.type == ItemType::Display)
        {
            canvas.DrawText(Rect{ctrlX - 40.0f, rect.position.y + 14.0f, 200.0f, 20.0f}, item.displayVal, Color{148, 163, 184, 255}, 13.0f);
        }
    }

    DrawFooterHints(canvas, width, height, "[UP/DOWN] Select  [LEFT/RIGHT] Adjust Slider/Picker  [ENTER] Toggle  [ESC] Back");
}

bool SettingsScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        auto& item = m_Items[m_SelectedIndex];

        switch (event.key.key)
        {
            case SDLK_UP:
                m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
                return true;
            case SDLK_DOWN:
                m_SelectedIndex = std::min(static_cast<int>(m_Items.size()) - 1, m_SelectedIndex + 1);
                return true;
            case SDLK_LEFT:
                if (item.type == ItemType::Slider)
                {
                    item.sliderVal = std::max(0.0f, item.sliderVal - 0.05f);
                }
                else if (item.type == ItemType::Picker && !item.options.empty())
                {
                    item.optionIdx = (item.optionIdx + item.options.size() - 1) % item.options.size();
                }
                return true;
            case SDLK_RIGHT:
                if (item.type == ItemType::Slider)
                {
                    item.sliderVal = std::min(1.0f, item.sliderVal + 0.05f);
                }
                else if (item.type == ItemType::Picker && !item.options.empty())
                {
                    item.optionIdx = (item.optionIdx + 1) % item.options.size();
                }
                return true;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
            case SDLK_SPACE:
                if (item.type == ItemType::Toggle)
                {
                    item.toggleVal = !item.toggleVal;
                }
                else if (item.type == ItemType::Picker && !item.options.empty())
                {
                    item.optionIdx = (item.optionIdx + 1) % item.options.size();
                }
                return true;
            default:
                break;
        }
    }

    return false;
}

// ===========================================================================
// FileBrowserScreen Implementation
// ===========================================================================
FileBrowserScreen::FileBrowserScreen()
    : AppScreen("File Browser", "Storage")
{
}

void FileBrowserScreen::NavigateTo(const std::string& path)
{
    if (path == "..")
    {
        auto pos = m_CurrentPath.find_last_of('/');
        if (pos != std::string::npos && pos > 0)
        {
            m_CurrentPath = m_CurrentPath.substr(0, pos);
        }
        else
        {
            m_CurrentPath = "/storage/media";
        }
    }
    else
    {
        std::string target = m_CurrentPath + "/" + path;
        if (m_FileSystem.find(target) != m_FileSystem.end())
        {
            m_CurrentPath = target;
        }
    }
    m_SelectedIndex = 0;
}

void FileBrowserScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    // Current Path Bar
    const Rect pathBar{24.0f, 92.0f, width - 48.0f, 38.0f};
    canvas.FillRoundedRect(pathBar, 6.0f, Color{14, 18, 26, 255});
    canvas.DrawRoundedRect(pathBar, 6.0f, Color{30, 38, 54, 255});

    if (m_PathLabelCache != m_CurrentPath)
    {
        m_PathLabel = "Path: " + m_CurrentPath;
        m_PathLabelCache = m_CurrentPath;
    }
    canvas.DrawText(Rect{pathBar.position.x + 14.0f, pathBar.position.y + 10.0f, pathBar.size.x - 28.0f, 20.0f}, m_PathLabel, Color{34, 211, 238, 255}, 15.0f);

    // Items List Table
    const auto it = m_FileSystem.find(m_CurrentPath);
    const std::vector<FsEntry>& items = (it != m_FileSystem.end()) ? it->second : m_FileSystem["/storage/media"];

    const float startY = 138.0f;
    const float itemH = 44.0f;
    const float gap = 6.0f;

    for (size_t i = 0; i < items.size(); ++i)
    {
        const Rect rect{24.0f, startY + i * (itemH + gap), width - 48.0f, itemH};
        const bool selected = (static_cast<int>(i) == m_SelectedIndex);

        const Color bg = selected ? Color{28, 35, 51, 255} : Color{18, 22, 32, 255};
        const Color border = selected ? Color{124, 58, 237, 255} : Color{30, 36, 51, 255};

        canvas.FillRoundedRect(rect, 6.0f, bg);
        canvas.DrawRoundedRect(rect, 6.0f, border);

        // Icon + Name
        canvas.DrawText(Rect{rect.position.x + 12.0f, rect.position.y + 10.0f, 24.0f, 20.0f}, items[i].icon, Color::White, 16.0f);
        canvas.DrawText(Rect{rect.position.x + 44.0f, rect.position.y + 12.0f, 240.0f, 20.0f}, items[i].name, selected ? Color::White : Color{226, 232, 240, 255}, 15.0f);

        // Size / Type & Date
        canvas.DrawText(Rect{rect.position.x + rect.size.x - 220.0f, rect.position.y + 12.0f, 90.0f, 20.0f}, items[i].sizeStr, Color{148, 163, 184, 255}, 13.0f);
        canvas.DrawText(Rect{rect.position.x + rect.size.x - 110.0f, rect.position.y + 12.0f, 90.0f, 20.0f}, items[i].dateStr, Color{100, 116, 139, 255}, 12.0f);
    }

    DrawFooterHints(canvas, width, height, "[UP/DOWN] Select  [ENTER] Open Folder  [BACKSPACE] Parent Dir  [ESC] Back");
}

bool FileBrowserScreen::HandleEvent(const SDL_Event& event)
{
    if (AppScreen::HandleEvent(event))
    {
        return true;
    }

    const auto it = m_FileSystem.find(m_CurrentPath);
    const auto& items = (it != m_FileSystem.end()) ? it->second : m_FileSystem["/storage/media"];

    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.key)
        {
            case SDLK_UP:
                m_SelectedIndex = std::max(0, m_SelectedIndex - 1);
                return true;
            case SDLK_DOWN:
                m_SelectedIndex = std::min(static_cast<int>(items.size()) - 1, m_SelectedIndex + 1);
                return true;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                if (!items.empty())
                {
                    const auto& entry = items[m_SelectedIndex];
                    if (entry.isDir)
                    {
                        NavigateTo(entry.name);
                    }
                }
                return true;
            case SDLK_BACKSPACE:
                NavigateTo("..");
                return true;
            default:
                break;
        }
    }

    return false;
}

// ===========================================================================
// PowerScreen Implementation
// ===========================================================================
PowerScreen::PowerScreen()
    : AppScreen("Power", "System")
{
}

void PowerScreen::OnUpdate(float deltaSeconds)
{
    if (m_Triggered)
    {
        m_Countdown -= deltaSeconds;
        if (m_Countdown <= 0.0f)
        {
            m_Countdown = 0.0f;
        }
    }
}

bool PowerScreen::NeedsRender() const
{
    return m_Triggered;
}

void PowerScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    AppScreen::Render(canvas, width, height);

    const float startY = 110.0f;
    const float itemH = 64.0f;
    const float gap = 12.0f;

    for (size_t i = 0; i < m_Options.size(); ++i)
    {
        const Rect rect{24.0f, startY + i * (itemH + gap), width - 48.0f, itemH};
        const bool selected = (static_cast<int>(i) == m_SelectedIndex);

        const auto& opt = m_Options[i];
        const Color bg = selected ? Color{28, 35, 51, 255} : Color{18, 22, 32, 255};
        const Color border = selected ? opt.color : Color{30, 36, 51, 255};

        canvas.FillRoundedRect(rect, 8.0f, bg);
        canvas.DrawRoundedRect(rect, 8.0f, border);

        canvas.DrawText(Rect{rect.position.x + 18.0f, rect.position.y + 18.0f, 30.0f, 28.0f}, opt.icon, opt.color, 22.0f);
        canvas.DrawText(Rect{rect.position.x + 58.0f, rect.position.y + 12.0f, 220.0f, 22.0f}, opt.name, selected ? Color::White : Color{226, 232, 240, 255}, 18.0f);
        canvas.DrawText(Rect{rect.position.x + 58.0f, rect.position.y + 36.0f, 300.0f, 18.0f}, opt.desc, Color{148, 163, 184, 255}, 13.0f);
    }

    // Triggered Action Dialog Overlay
    if (m_Triggered)
    {
        canvas.FillRect(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)}, Color{0, 0, 0, 230});

        const Rect modal{100.0f, 140.0f, width - 200.0f, 180.0f};
        canvas.FillRoundedRect(modal, 12.0f, Color{18, 24, 36, 255});
        canvas.DrawRoundedRect(modal, 12.0f, Color{239, 68, 68, 255});

        canvas.DrawText(Rect{modal.position.x + 30.0f, modal.position.y + 30.0f, modal.size.x - 60.0f, 28.0f}, m_TriggeredAction + " in progress", Color::White, 22.0f);

        char cdBuf[64];
        std::snprintf(cdBuf, sizeof(cdBuf), "Execution in %.1f seconds...", m_Countdown);
        canvas.DrawText(Rect{modal.position.x + 30.0f, modal.position.y + 75.0f, modal.size.x - 60.0f, 22.0f}, cdBuf, Color{34, 211, 238, 255}, 16.0f);
        canvas.DrawText(Rect{modal.position.x + 30.0f, modal.position.y + 120.0f, modal.size.x - 60.0f, 20.0f}, "Press [ESC] to cancel", Color{148, 163, 184, 255}, 14.0f);
    }

    DrawFooterHints(canvas, width, height, "[UP/DOWN] Select Action  [ENTER] Confirm  [ESC] Back");
}

bool PowerScreen::HandleEvent(const SDL_Event& event)
{
    if (m_Triggered)
    {
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        {
            m_Triggered = false;
            return true;
        }
        return true;
    }

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
                m_SelectedIndex = std::min(static_cast<int>(m_Options.size()) - 1, m_SelectedIndex + 1);
                return true;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                if (m_SelectedIndex == 3 && m_OnBack)
                {
                    m_OnBack();
                }
                else
                {
                    m_TriggeredAction = m_Options[m_SelectedIndex].name;
                    m_Triggered = true;
                    m_Countdown = 3.0f;
                }
                return true;
            default:
                break;
        }
    }

    return false;
}

} // namespace flachead::apps
