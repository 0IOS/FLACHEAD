#pragma once

#include "ShellScreen.hpp"

#include <string_view>
#include <vector>

namespace flachead::shell
{
// The task overview: a vertically scrollable stack of open screens. Tapping a
// card pops to that screen; a system row offers Home / Search / Settings.
class TaskOverviewScreen : public ShellScreen
{
public:
    explicit TaskOverviewScreen(const ShellServices& services);

protected:
    void BuildShell() override;
    void OnShellUpdate(float deltaSeconds) override;
    bool OnShellCommand(flachead::commands::Command command) override;
    std::string_view ScreenKey() const override { return "taskoverview"; }

private:
    void BuildOverview();
    void BuildEmpty();
};
} // namespace flachead::shell
