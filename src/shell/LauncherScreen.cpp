#include "LauncherScreen.hpp"

#include "../ui/Button.hpp"

namespace flachead::shell
{
LauncherScreen::LauncherScreen(const ShellServices& services)
    : ShellScreen(services)
{
}

const std::vector<LauncherScreen::App>& LauncherScreen::Apps()
{
    static const std::vector<App> kApps = {
        {"Now Playing", "nowplaying"},
        {"Songs", "songs"},
        {"Albums", "albums"},
        {"Artists", "artists"},
        {"Favorites", "favorites"},
        {"Recent", "recent"},
        {"Playlists", "playlists"},
        {"Search", "universal_search"},
        {"Settings", "dapsettings"},
        {"Queue", "queue"},
    };
    return kApps;
}

void LauncherScreen::BuildShell()
{
    auto& root = Root();
    root.SetLayout(flachead::layout::MakeGrid(2));

    for (const App& app : Apps())
    {
        auto tile = std::make_unique<flachead::ui::Button>();
        tile->SetId(std::string{app.screen});
        tile->SetText(std::string{app.name});
        tile->SetFocusable(true);
        tile->SetClickHandler([this, screen = app.screen] { LaunchApp(screen); });
        root.AddChild(std::move(tile));
    }
}

void LauncherScreen::LaunchApp(std::string_view screenName)
{
    Ctx().navigate(screenName);
}

bool LauncherScreen::OnShellCommand(flachead::commands::Command command)
{
    switch (command)
    {
        case flachead::commands::Command::Back:
            Ctx().goBack();
            return true;
        default:
            return false;
    }
}

void LauncherScreen::OnShellInput(const flachead::input::InputEvent& event)
{
    if (event.action == flachead::input::InputAction::Swipe && event.delta.y > 0.0f)
    {
        Ctx().goBack();
    }
}
} // namespace flachead::shell
