#include "SettingsScreen.hpp"

#include "../events/EventBus.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"
#include "../playback/QueueManager.hpp"

#include <algorithm>
#include <cstdio>

namespace flachead::dap
{
namespace
{
const Color kAccent  = Color{124, 58, 237, 255};
const Color kFg      = Color{226, 232, 240, 255};
const Color kFgMuted = Color{148, 163, 184, 255};
const Color kLine    = Color{30, 36, 51, 255};
const Color kRowSel  = Color{28, 35, 51, 255};
const Color kRowBg   = Color{16, 20, 30, 255};
constexpr float kRowH = 58.0f;
constexpr float kListTop = kStatusH + kHeaderH + 6.0f;

const char* kRepeatNames[] = {"Off", "All", "One"};
} // namespace

SettingsScreen::SettingsScreen(const AppContext& context)
    : DapScreen(context, "Settings")
{
    Subscribe(flachead::events::Type::SettingsChanged);
    Subscribe(flachead::events::Type::VolumeChanged);
    Subscribe(flachead::events::Type::ShuffleChanged);
    Subscribe(flachead::events::Type::RepeatChanged);
    Subscribe(flachead::events::Type::LibraryScanFinished);
}

void SettingsScreen::RefreshData()
{
    m_ScanFolder = Ctx().scanRoots.empty() ? "" : Ctx().scanRoots[0];
}

void SettingsScreen::CommitVolume()
{
    Ctx().settings->SetFloat("audio.volume", Ctx().playback->Volume());
}

void SettingsScreen::CommitScanFolder()
{
    if (m_ScanFolder.empty())
    {
        return;
    }
    Ctx().settings->Set("library.scan_roots", m_ScanFolder);
    Ctx().scanRoots = {m_ScanFolder};
    Ctx().library->StartScan(Ctx().scanRoots);
}

void SettingsScreen::CycleRepeat()
{
    auto& playback = *Ctx().playback;
    const auto current = playback.Queue().Repeat();
    const auto next = static_cast<flachead::playback::RepeatMode>(
        (static_cast<int>(current) + 1) % 3);
    playback.SetRepeat(next);
    Ctx().settings->SetInt("playback.repeat", static_cast<int>(next));
}

void SettingsScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    UpdateViewSize(width, height);
    DrawBackground(canvas, width, height);
    DrawStatusBar(canvas, width);
    DrawHeader(canvas, width, Title());

    auto& playback = *Ctx().playback;
    auto& settings = *Ctx().settings;

    const float maxY = static_cast<float>(height) - kFooterH - 6.0f;
    const int visible = std::max(1, static_cast<int>((maxY - kListTop) / kRowH));
    const int startRow = std::max(0, static_cast<int>(m_SelectedRow) - visible / 2);
    const int endRow = std::min(6, startRow + visible);

    for (int r = startRow; r < endRow; ++r)
    {
        const Row row = static_cast<Row>(r);
        const bool selected = row == m_SelectedRow;
        const float y = kListTop + static_cast<float>(r - startRow) * kRowH;

        canvas.FillRect(Rect{0.0f, y, static_cast<float>(width), kRowH},
                        selected ? kRowSel : kRowBg);
        if (selected)
        {
            canvas.FillRect(Rect{0.0f, y, 4.0f, kRowH}, kAccent);
        }
        canvas.DrawLine(0.0f, y + kRowH, static_cast<float>(width), y + kRowH, kLine);

        const float labelX = 22.0f;
        const float valueX = static_cast<float>(width) - 300.0f;

        switch (row)
        {
            case Row::Volume:
            {
                canvas.DrawText(Rect{labelX, y + 8.0f, 200.0f, 20.0f}, "Volume",
                                selected ? Color::White : kFg, 16.0f);
                canvas.DrawText(Rect{valueX, y + 10.0f, 260.0f, 18.0f}, "LEFT/RIGHT to adjust",
                                kFgMuted, 12.0f);
                const float vol = playback.Volume();
                constexpr float barW = 200.0f;
                canvas.FillRoundedRect(Rect{valueX, y + 30.0f, barW, 6.0f}, 3.0f,
                                       Color{22, 28, 40, 255});
                if (vol > 0.0f)
                {
                    canvas.FillRoundedRect(Rect{valueX, y + 30.0f, barW * vol, 6.0f}, 3.0f,
                                           Color{34, 211, 238, 255});
                }
                char buf[8];
                std::snprintf(buf, sizeof(buf), "%d%%", static_cast<int>(vol * 100.0f + 0.5f));
                canvas.DrawText(Rect{valueX + barW + 14.0f, y + 24.0f, 50.0f, 18.0f}, buf,
                                kFgMuted, 14.0f);
                break;
            }
            case Row::ScanFolder:
            {
                canvas.DrawText(Rect{labelX, y + 8.0f, 220.0f, 20.0f}, "Scan folder",
                                selected ? Color::White : kFg, 16.0f);
                const std::string value = m_EditingFolder ? m_ScanFolder + "|"
                                                          : (m_ScanFolder.empty()
                                                                 ? "<not set>"
                                                                 : m_ScanFolder);
                canvas.DrawText(Rect{valueX, y + 10.0f, 270.0f, 18.0f}, value,
                                m_ScanFolder.empty() ? kFgMuted : Color{203, 213, 225, 255},
                                m_EditingFolder ? 15.0f : 13.0f);
                canvas.DrawText(Rect{labelX, y + 32.0f, 400.0f, 16.0f},
                                m_EditingFolder ? "Type to edit, ENTER to save & rescan"
                                                : "ENTER to edit",
                                kFgMuted, 12.0f);
                break;
            }
            case Row::DefaultShuffle:
            {
                canvas.DrawText(Rect{labelX, y + 8.0f, 220.0f, 20.0f}, "Default shuffle",
                                selected ? Color::White : kFg, 16.0f);
                const bool on = settings.GetBool("playback.shuffle", false);
                canvas.DrawText(Rect{valueX, y + 12.0f, 60.0f, 20.0f}, on ? "ON" : "OFF",
                                on ? Color{52, 211, 153, 255} : kFgMuted, 16.0f);
                canvas.DrawText(Rect{valueX + 70.0f, y + 16.0f, 200.0f, 16.0f},
                                "ENTER to toggle", kFgMuted, 12.0f);
                break;
            }
            case Row::DefaultRepeat:
            {
                canvas.DrawText(Rect{labelX, y + 8.0f, 220.0f, 20.0f}, "Default repeat",
                                selected ? Color::White : kFg, 16.0f);
                const int idx = std::clamp(settings.GetInt("playback.repeat", 0), 0, 2);
                canvas.DrawText(Rect{valueX, y + 12.0f, 80.0f, 20.0f}, kRepeatNames[idx],
                                Color{203, 213, 225, 255}, 16.0f);
                canvas.DrawText(Rect{valueX + 90.0f, y + 16.0f, 200.0f, 16.0f},
                                "ENTER to cycle", kFgMuted, 12.0f);
                break;
            }
            case Row::Rescan:
            {
                canvas.DrawText(Rect{labelX, y + 8.0f, 240.0f, 20.0f}, "Rescan library",
                                selected ? Color::White : kFg, 16.0f);
                canvas.DrawText(Rect{labelX, y + 32.0f, 400.0f, 16.0f},
                                "Imports new files and prunes deleted ones", kFgMuted, 12.0f);
                break;
            }
            case Row::About:
            {
                canvas.DrawText(Rect{labelX, y + 8.0f, 200.0f, 20.0f}, "About",
                                selected ? Color::White : kFg, 16.0f);
                std::string info = "FLACHEAD v0.1.0  ·  backend: ";
                info += std::string(Ctx().playback->BackendName());
                canvas.DrawText(Rect{valueX, y + 12.0f, 270.0f, 18.0f}, info, kFgMuted, 13.0f);
                break;
            }
        }
    }

    DrawFooter(canvas, width, height,
               "UP/DOWN: select   ENTER/SPACE: activate   LEFT/RIGHT: adjust   ESC: back");
}

bool SettingsScreen::HandleEvent(const SDL_Event& event)
{
    if (event.type != SDL_EVENT_KEY_DOWN)
    {
        return false;
    }

    auto& playback = *Ctx().playback;
    auto& settings = *Ctx().settings;
    const SDL_Keycode key = event.key.key;

    if (m_EditingFolder)
    {
        switch (key)
        {
            case SDLK_ESCAPE:
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                m_EditingFolder = false;
                CommitScanFolder();
                return true;
            case SDLK_BACKSPACE:
                if (!m_ScanFolder.empty())
                {
                    m_ScanFolder.pop_back();
                }
                return true;
            default:
                if (key >= ' ' && key <= '~' && m_ScanFolder.size() < 200)
                {
                    m_ScanFolder += static_cast<char>(key);
                }
                return true;
        }
    }

    switch (key)
    {
        case SDLK_ESCAPE:
            Ctx().goBack();
            return true;
        case SDLK_UP:
            if (m_SelectedRow != Row::Volume)
            {
                m_SelectedRow = static_cast<Row>(static_cast<int>(m_SelectedRow) - 1);
            }
            return true;
        case SDLK_DOWN:
            if (m_SelectedRow != Row::About)
            {
                m_SelectedRow = static_cast<Row>(static_cast<int>(m_SelectedRow) + 1);
            }
            return true;
        case SDLK_LEFT:
            if (m_SelectedRow == Row::Volume)
            {
                playback.SetVolume(playback.Volume() - 0.05f);
                CommitVolume();
            }
            return true;
        case SDLK_RIGHT:
            if (m_SelectedRow == Row::Volume)
            {
                playback.SetVolume(playback.Volume() + 0.05f);
                CommitVolume();
            }
            return true;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            switch (m_SelectedRow)
            {
                case Row::Volume:
                    break;
                case Row::ScanFolder:
                    m_EditingFolder = true;
                    break;
                case Row::DefaultShuffle:
                {
                    playback.ToggleShuffle();
                    settings.SetBool("playback.shuffle", playback.Queue().Shuffle());
                    break;
                }
                case Row::DefaultRepeat:
                    CycleRepeat();
                    break;
                case Row::Rescan:
                    Ctx().library->StartScan(Ctx().scanRoots);
                    break;
                case Row::About:
                    break;
            }
            return true;
        default:
            break;
    }
    return false;
}
} // namespace flachead::dap
