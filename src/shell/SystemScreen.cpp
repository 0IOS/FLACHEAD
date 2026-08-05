#include "SystemScreen.hpp"

#include "../ui/Button.hpp"
#include "../ui/Label.hpp"

#include <cstdio>

namespace flachead::shell
{
SystemScreen::SystemScreen(const ShellServices& services)
    : ShellScreen(services)
{
}

void SystemScreen::BuildShell()
{
    auto& root = Root();
    root.SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Vertical));

    auto title = std::make_unique<flachead::ui::Label>();
    title->SetText("About FLACHEAD");
    title->SetFontSize(18.0f);
    title->SetAlign(flachead::ui::Label::Align::Left);
    root.AddChild(std::move(title));

    const auto addInfo = [this](std::string id, std::string_view text) {
        auto button = std::make_unique<flachead::ui::Button>();
        button->SetId(id);
        button->SetText(text);
        button->SetFocusable(false);
        button->SetBackground(Color{16, 18, 26});
        m_InfoButtonIds.push_back(std::string(id));
        Root().AddChild(std::move(button));
    };

    addInfo("info.version", "Version 0.1.0");
    addInfo("info.backend", "Backend: --");
    addInfo("info.tracks", "Tracks: --");
    addInfo("info.settings", "Scan roots: --");
    addInfo("info.memory", "Memory: --");

    auto spacer = std::make_unique<flachead::ui::Label>();
    spacer->SetText("");
    spacer->SetFontSize(6.0f);
    root.AddChild(std::move(spacer));

    auto restart = std::make_unique<flachead::ui::Button>();
    restart->SetId("power.restart");
    restart->SetText("Restart player");
    restart->SetFocusable(true);
    restart->SetAccentColor(Color{245, 158, 11});
    restart->SetClickHandler([this] {
        if (Services().quit)
        {
            Services().quit();
        }
    });
    root.AddChild(std::move(restart));

    auto shutdown = std::make_unique<flachead::ui::Button>();
    shutdown->SetId("power.shutdown");
    shutdown->SetText("Shut down");
    shutdown->SetFocusable(true);
    shutdown->SetAccentColor(Color{239, 68, 68});
    shutdown->SetClickHandler([this] {
        if (Services().quit)
        {
            Services().quit();
        }
    });
    root.AddChild(std::move(shutdown));

    RefreshLabels();
}

void SystemScreen::RefreshLabels()
{
    for (auto& child : Root().Children())
    {
        flachead::ui::Button* button = dynamic_cast<flachead::ui::Button*>(child.get());
        if (!button || button->Id().empty())
        {
            continue;
        }
        if (button->Id() == "info.backend")
        {
            button->SetText("Backend: " + std::string(Ctx().playback ? Ctx().playback->BackendName() : "--"));
        }
        else if (button->Id() == "info.tracks")
        {
            std::size_t count = 0;
            if (Ctx().library)
            {
                count = static_cast<std::size_t>(std::max(0, Ctx().library->SongCount()));
            }
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "Tracks: %zu", count);
            button->SetText(buffer);
        }
        else if (button->Id() == "info.memory")
        {
            char buffer[96];
            if (Services().memory && Services().memory->CurrentRssKb() > 0)
            {
                std::snprintf(buffer, sizeof(buffer), "Memory: %zu / %zu KB",
                              Services().memory->CurrentRssKb(), Services().memory->PeakRssKb());
            }
            else
            {
                std::snprintf(buffer, sizeof(buffer), "Memory: --");
            }
            button->SetText(buffer);
        }
        else if (button->Id() == "info.settings")
        {
            std::string roots;
            for (const auto& root : Ctx().scanRoots)
            {
                if (!roots.empty())
                {
                    roots += ", ";
                }
                roots += root;
            }
            button->SetText("Scan roots: " + roots);
        }
    }
}

void SystemScreen::OnShellUpdate(float deltaSeconds)
{
    m_RefreshAccumulator += deltaSeconds;
    if (m_RefreshAccumulator >= 0.5f)
    {
        m_RefreshAccumulator = 0.0f;
        RefreshLabels();
        MarkDirty();
    }
}

bool SystemScreen::OnShellCommand(flachead::commands::Command command)
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
} // namespace flachead::shell
