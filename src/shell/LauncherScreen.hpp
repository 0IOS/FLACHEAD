#pragma once

#include "ShellScreen.hpp"

#include <string_view>
#include <vector>

namespace flachead::shell
{
// The app launcher: a grid of library and system apps. Tapping a tile pushes
// that screen; the launcher stays beneath so home always returns.
class LauncherScreen : public ShellScreen
{
public:
    explicit LauncherScreen(const ShellServices& services);

protected:
    void BuildShell() override;
    void OnShellEnter() override;
    bool OnShellCommand(flachead::commands::Command command) override;
    void OnShellInput(const flachead::input::InputEvent& event) override;
    std::string_view ScreenKey() const override { return "launcher"; }

private:
    void LaunchApp(std::string_view screenName);

    struct App
    {
        std::string_view name;
        std::string_view screen;
    };
    static const std::vector<App>& Apps();
};
} // namespace flachead::shell
