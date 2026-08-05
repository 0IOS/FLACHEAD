#pragma once

#include "ShellScreen.hpp"

#include "../models/SongModel.hpp"

#include <string_view>
#include <vector>

namespace flachead::ui
{
class Label;
class TextField;
} // namespace flachead::ui

namespace flachead::shell
{
// Universal search: type with the on-screen keyboard (or hardware keys) and
// get matching songs; tapping a result plays it. With an empty query it shows
// recent tracks as quick picks.
class UniversalSearchScreen : public ShellScreen
{
public:
    explicit UniversalSearchScreen(const ShellServices& services);

protected:
    void BuildShell() override;
    void OnShellUpdate(float deltaSeconds) override;
    bool OnShellCommand(flachead::commands::Command command) override;
    std::string_view ScreenKey() const override { return "universal_search"; }

private:
    void RefreshResults();
    void RebuildResults(const std::vector<flachead::models::SongModel>& songs);
    void AppendChar(char ch);
    void Backspace();
    void PlayTrack(const flachead::models::SongModel& song);

    flachead::ui::TextField* m_Field{nullptr};
    flachead::ui::Label* m_Status{nullptr};
    flachead::ui::Widget* m_Results{nullptr};
    std::vector<flachead::models::SongModel> m_Songs;
};
} // namespace flachead::shell
