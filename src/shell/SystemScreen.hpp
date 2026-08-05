#pragma once

#include "ShellScreen.hpp"

namespace flachead::shell
{
// System information and power actions. The About block reads live service
// state (audio backend, library counts); the power row requests an
// application exit through the shell quit hook.
class SystemScreen : public ShellScreen
{
public:
    explicit SystemScreen(const ShellServices& services);

protected:
    void BuildShell() override;
    void OnShellUpdate(float deltaSeconds) override;
    bool OnShellCommand(flachead::commands::Command command) override;
    std::string_view ScreenKey() const override { return "system"; }

private:
    void RefreshLabels();

    std::vector<std::string> m_InfoButtonIds;
};
} // namespace flachead::shell
