#pragma once

#include "ShellScreen.hpp"

namespace flachead::ui
{
class Slider;
} // namespace flachead::ui

namespace flachead::shell
{
// Operating-environment settings: volume, playback defaults, theme accent,
// library rescan. Values are read from / written back to the SettingsManager
// so they persist across restarts.
class SettingsScreen : public ShellScreen
{
public:
    explicit SettingsScreen(const ShellServices& services);

protected:
    void BuildShell() override;
    void OnShellUpdate(float deltaSeconds) override;
    bool OnShellCommand(flachead::commands::Command command) override;
    std::string_view ScreenKey() const override { return "settings"; }

private:
    void ApplyAccent(const Color& color);
    void ToggleDark();
    void ToggleShuffle();
    void CycleRepeat();
    void Rescan();
    void RefreshThemeButtons();

    flachead::ui::Slider* m_Volume{nullptr};
    bool m_Dark{true};
    std::vector<std::string> m_AccentButtons;
};
} // namespace flachead::shell
