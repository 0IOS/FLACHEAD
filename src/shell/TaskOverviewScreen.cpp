#include "TaskOverviewScreen.hpp"

#include "../ui/Button.hpp"
#include "../ui/Label.hpp"

#include <string>
#include <utility>

namespace flachead::shell
{
namespace
{
std::string FriendlyName(std::string_view key)
{
    static const std::vector<std::pair<std::string_view, std::string_view>> kNames = {
        {"nowplaying", "Now Playing"}, {"songs", "Songs"},   {"albums", "Albums"},
        {"artists", "Artists"},       {"favorites", "Favorites"}, {"recent", "Recent"},
        {"playlists", "Playlists"},   {"universal_search", "Search"}, {"settings", "Settings"},
        {"launcher", "Launcher"},     {"queue", "Queue"},
    };
    for (const auto& [from, to] : kNames)
    {
        if (from == key)
        {
            return std::string{to};
        }
    }
    return std::string{key};
}
} // namespace

TaskOverviewScreen::TaskOverviewScreen(const ShellServices& services)
    : ShellScreen(services)
{
}

void TaskOverviewScreen::BuildShell()
{
    BuildOverview();
}

void TaskOverviewScreen::BuildOverview()
{
    auto& root = Root();

    auto heading = std::make_unique<flachead::ui::Label>();
    heading->SetText("Open apps");
    heading->SetFontSize(16.0f);
    heading->SetAlign(flachead::ui::Label::Align::Left);
    root.AddChild(std::move(heading));

    const auto& stack = Services().screens ? Services().screens->Stack() : std::vector<std::string>{};
    std::vector<std::string> tasks;
    for (auto it = stack.rbegin(); it != stack.rend(); ++it)
    {
        if (*it != "taskoverview")
        {
            tasks.push_back(*it);
        }
    }
    if (tasks.empty())
    {
        auto empty = std::make_unique<flachead::ui::Label>();
        empty->SetText("No open apps");
        empty->SetFontSize(13.0f);
        empty->SetAlign(flachead::ui::Label::Align::Center);
        root.AddChild(std::move(empty));
    }
    else
    {
        for (const auto& task : tasks)
        {
            auto card = std::make_unique<flachead::ui::Button>();
            card->SetId("task_" + task);
            card->SetText(FriendlyName(task));
            card->SetFocusable(true);
            card->SetClickHandler([this, key = task] { Services().screens->PopTo(key); });
            root.AddChild(std::move(card));
        }
    }

    auto systemRow = std::make_unique<flachead::ui::Container>();
    systemRow->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Horizontal));
    flachead::ui::Widget* row = systemRow.get();
    root.AddChild(std::move(systemRow));

    auto home = std::make_unique<flachead::ui::Button>();
    home->SetId("home");
    home->SetText("Home");
    home->SetFocusable(true);
    home->SetClickHandler([this] { Services().screens->PopTo("home"); });
    row->AddChild(std::move(home));

    auto search = std::make_unique<flachead::ui::Button>();
    search->SetId("search");
    search->SetText("Search");
    search->SetFocusable(true);
    search->SetClickHandler([this] { Ctx().navigate("universal_search"); });
    row->AddChild(std::move(search));

    auto settings = std::make_unique<flachead::ui::Button>();
    settings->SetId("settings");
    settings->SetText("Settings");
    settings->SetFocusable(true);
    settings->SetClickHandler([this] { Ctx().navigate("settings"); });
    row->AddChild(std::move(settings));
}

void TaskOverviewScreen::BuildEmpty()
{
    BuildOverview();
}

void TaskOverviewScreen::OnShellUpdate(float deltaSeconds)
{
    (void)deltaSeconds;
}

bool TaskOverviewScreen::OnShellCommand(flachead::commands::Command command)
{
    if (command == flachead::commands::Command::Back)
    {
        Ctx().goBack();
        return true;
    }
    return false;
}
} // namespace flachead::shell
