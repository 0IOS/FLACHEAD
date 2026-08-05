#pragma once

#include "../models/SongModel.hpp"
#include "DapScreen.hpp"

#include <string>
#include <vector>

namespace flachead::dap
{
// Full-screen player: artwork, metadata, seek bar, transport controls,
// volume, favorite and queue access. Keyboard: SPACE/ENTER play-pause,
// LEFT/RIGHT seek, UP/DOWN volume, P/N prev/next, S/R shuffle/repeat,
// F favorite, Q queue.
class NowPlayingScreen : public DapScreen
{
public:
    explicit NowPlayingScreen(const AppContext& context);

    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    void DrawArt(flachead::ui::Canvas& canvas, int width) const;
    void DrawTransport(flachead::ui::Canvas& canvas, int width, int height) const;
    void DrawVolume(flachead::ui::Canvas& canvas, int width, int height) const;
    void ToggleFavorite();

    int m_Focus{0};
};

// Queue / "Up Next" manager: list the queued tracks, remove or clear entries,
// and inspect repeat/shuffle. ENTER removes the selected row, C clears.
class QueueScreen : public DapScreen
{
public:
    explicit QueueScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    std::vector<flachead::models::SongModel> m_Tracks;
    int m_SelectedIndex{0};
};

// Library scanner status + control. Shows incremental progress while a scan
// runs and offers Start/Cancel actions. SPACE/ENTER activates the focused
// action.
class ScanScreen : public DapScreen
{
public:
    explicit ScanScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    enum class Action { Start, Cancel };

    void Activate();

    bool m_Scanning{false};
    int m_Processed{0};
    int m_Total{0};
    int m_LastCount{0};
    Action m_Focus{Action::Start};
};
} // namespace flachead::dap
