#include "LauncherScreen.hpp"

#include "../ui/Button.hpp"
#include "../ui/Label.hpp"
#include "../ui/Container.hpp"
#include "../layout/LayoutEngine.hpp"

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
        {"Settings", "settings"},
        {"Queue", "queue"},
    };
    return kApps;
}

void LauncherScreen::BuildShell()
{
    auto& root = Root();
    root.SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Vertical, 18.0f, 18.0f));

    auto title = std::make_unique<flachead::ui::Label>();
    title->SetText("Launcher");
    title->SetFontSize(20.0f);
    title->SetAlign(flachead::ui::Label::Align::Left);
    root.AddChild(std::move(title));

    auto subtitle = std::make_unique<flachead::ui::Label>();
    subtitle->SetText("Browse your music and system screens");
    subtitle->SetFontSize(12.0f);
    subtitle->SetAlign(flachead::ui::Label::Align::Left);
    root.AddChild(std::move(subtitle));

    auto grid = std::make_unique<flachead::ui::Container>();
    grid->SetLayout(flachead::layout::MakeGrid(3, 14.0f));
    flachead::ui::Widget* gridRaw = grid.get();
    root.AddChild(std::move(grid));

    const auto& palette = Themes().ActivePalette();
    for (const App& app : Apps())
    {
        auto tile = std::make_unique<flachead::ui::Button>();
        tile->SetId(std::string{app.screen});
        tile->SetText(std::string{app.name});
        tile->SetFocusable(true);
        tile->SetAccentColor(palette.accent);
        tile->SetBackground(palette.surfaceRaised);
        tile->SetClickHandler([this, screen = app.screen] { LaunchApp(screen); });
        gridRaw->AddChild(std::move(tile));
    }
}

void LauncherScreen::OnShellEnter()
{
    const auto& palette = Themes().ActivePalette();
    Wallpaper().SetTint(palette.background, 0.16f);
    Wallpaper().SetDim(0.58f);
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
