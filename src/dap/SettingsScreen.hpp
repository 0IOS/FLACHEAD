#pragma once

#include "DapScreen.hpp"

#include <string>
#include <vector>

namespace flachead::dap
{
// Player settings: volume, default shuffle/repeat, scan folder, library
// rescan and device info. Volume is live (applies to playback + is persisted);
// the scan folder row supports inline editing (ENTER to edit, ENTER to commit).
class SettingsScreen : public DapScreen
{
public:
    explicit SettingsScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

private:
    enum class Row
    {
        Volume,
        ScanFolder,
        DefaultShuffle,
        DefaultRepeat,
        Rescan,
        About,
    };

    void CommitVolume();
    void CommitScanFolder();
    void CycleRepeat();

    std::string m_ScanFolder;
    bool m_EditingFolder{false};
    Row m_SelectedRow{Row::Volume};
};
} // namespace flachead::dap
