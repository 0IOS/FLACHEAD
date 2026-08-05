#include "SettingsScreen.hpp"

#include "../playback/PlaybackController.hpp"
#include "../ui/Button.hpp"
#include "../ui/Label.hpp"
#include "../ui/Slider.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace flachead::shell
{
namespace
{
constexpr const char* kRepeatNames[] = {"Off", "All", "One"};
const char* RepeatName(int index)
{
    return kRepeatNames[std::clamp(index, 0, 2)];
}

const Color kAccents[] = {
    {124, 58, 237},  // violet
    {34, 211, 238},  // cyan
    {239, 68, 68},   // red
    {34, 197, 94},   // green
    {245, 158, 11},  // amber
    {236, 72, 153},  // pink
};
} // namespace

SettingsScreen::SettingsScreen(const ShellServices& services)
    : ShellScreen(services)
{
}

void SettingsScreen::BuildShell()
{
    auto& root = Root();
    root.SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Vertical));

    m_Dark = Themes().IsDark();

    auto volumeRow = std::make_unique<flachead::ui::Container>();
    volumeRow->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Vertical));
    flachead::ui::Widget* volumeRowRaw = volumeRow.get();
    root.AddChild(std::move(volumeRow));

    auto volumeLabel = std::make_unique<flachead::ui::Label>();
    volumeLabel->SetText("Volume");
    volumeLabel->SetFontSize(13.0f);
    volumeLabel->SetAlign(flachead::ui::Label::Align::Left);
    volumeRowRaw->AddChild(std::move(volumeLabel));

    m_Volume = new flachead::ui::Slider();
    m_Volume->SetId("volume");
    m_Volume->SetRange(0.0f, 1.0f);
    m_Volume->SetValue(Ctx().playback ? Ctx().playback->Volume() : 0.8f);
    m_Volume->SetFocusable(true);
    m_Volume->SetChangeCallback([this](float value) {
        if (Ctx().playback)
        {
            Ctx().playback->SetVolume(value);
        }
        if (Ctx().settings)
        {
            Ctx().settings->SetFloat("audio.volume", value);
        }
        MarkDirty();
    });
    auto volume = std::unique_ptr<flachead::ui::Slider>(m_Volume);
    volumeRowRaw->AddChild(std::move(volume));

    auto shuffle = std::make_unique<flachead::ui::Button>();
    shuffle->SetId("shuffle");
    shuffle->SetFocusable(true);
    shuffle->SetClickHandler([this] { ToggleShuffle(); });
    root.AddChild(std::move(shuffle));

    auto repeat = std::make_unique<flachead::ui::Button>();
    repeat->SetId("repeat");
    repeat->SetFocusable(true);
    repeat->SetClickHandler([this] { CycleRepeat(); });
    root.AddChild(std::move(repeat));

    auto theme = std::make_unique<flachead::ui::Container>();
    theme->SetLayout(flachead::layout::MakeGrid(3));
    flachead::ui::Widget* themeRaw = theme.get();
    root.AddChild(std::move(theme));

    m_AccentButtons.clear();
    for (std::size_t i = 0; i < sizeof(kAccents) / sizeof(kAccents[0]); ++i)
    {
        const auto id = "accent_" + std::to_string(i);
        auto tile = std::make_unique<flachead::ui::Button>();
        tile->SetId(id);
        tile->SetText(std::to_string(i + 1));
        tile->SetAccentColor(kAccents[i]);
        tile->SetFocusable(true);
        tile->SetClickHandler([this, i] { ApplyAccent(kAccents[i]); });
        m_AccentButtons.push_back(id);
        themeRaw->AddChild(std::move(tile));
    }

    auto dark = std::make_unique<flachead::ui::Button>();
    dark->SetId("dark");
    dark->SetFocusable(true);
    dark->SetClickHandler([this] { ToggleDark(); });
    root.AddChild(std::move(dark));

    auto rescan = std::make_unique<flachead::ui::Button>();
    rescan->SetId("rescan");
    rescan->SetFocusable(true);
    rescan->SetClickHandler([this] { Rescan(); });
    root.AddChild(std::move(rescan));

    auto about = std::make_unique<flachead::ui::Button>();
    about->SetId("about");
    about->SetFocusable(true);
    about->SetClickHandler([this] { Ctx().navigate("system"); });
    root.AddChild(std::move(about));

    RefreshThemeButtons();
}

void SettingsScreen::ApplyAccent(const Color& color)
{
    Themes().ApplySeed(color, m_Dark);
    if (Ctx().settings)
    {
        char hex[9];
        std::snprintf(hex, sizeof(hex), "#%02x%02x%02x", color.r, color.g, color.b);
        Ctx().settings->Set("theme.accent", hex);
        Ctx().settings->SetBool("theme.dark", m_Dark);
    }
    RefreshThemeButtons();
    MarkDirty();
}

void SettingsScreen::ToggleDark()
{
    m_Dark = !m_Dark;
    const auto accent = Themes().ActivePalette().accent;
    ApplyAccent(accent);
}

void SettingsScreen::ToggleShuffle()
{
    if (!Ctx().playback)
    {
        return;
    }
    Ctx().playback->ToggleShuffle();
    if (Ctx().settings)
    {
        Ctx().settings->SetBool("playback.shuffle", Ctx().playback->Queue().Shuffle());
    }
    MarkDirty();
}

void SettingsScreen::CycleRepeat()
{
    if (!Ctx().playback)
    {
        return;
    }
    Ctx().playback->ToggleRepeat();
    if (Ctx().settings)
    {
        Ctx().settings->SetInt("playback.repeat", static_cast<int>(Ctx().playback->Queue().Repeat()));
    }
    MarkDirty();
}

void SettingsScreen::Rescan()
{
    if (Ctx().library && !Ctx().scanRoots.empty())
    {
        Ctx().library->StartScan(Ctx().scanRoots);
        auto toast = std::make_shared<flachead::ui::Label>();
        toast->SetText("Scan started");
        Overlays().PushToast("scan", std::move(toast));
    }
    MarkDirty();
}

void SettingsScreen::RefreshThemeButtons()
{
    if (!m_Volume)
    {
        return;
    }
    m_Volume->SetValue(Ctx().playback ? Ctx().playback->Volume() : 0.8f);

    const bool shuffle = Ctx().playback && Ctx().playback->Queue().Shuffle();
    const int repeat = Ctx().playback ? static_cast<int>(Ctx().playback->Queue().Repeat()) : 0;
    for (auto& child : Root().Children())
    {
        flachead::ui::Button* button = dynamic_cast<flachead::ui::Button*>(child.get());
        if (!button)
        {
            continue;
        }
        if (button->Id() == "shuffle")
        {
            button->SetText(std::string("Shuffle: ") + (shuffle ? "On" : "Off"));
        }
        else if (button->Id() == "repeat")
        {
            button->SetText(std::string("Repeat: ") + RepeatName(repeat));
        }
        else if (button->Id() == "dark")
        {
            button->SetText(std::string("Theme: ") + (m_Dark ? "Dark" : "Light"));
        }
        else if (button->Id() == "rescan")
        {
            button->SetText("Rescan library");
        }
        else if (button->Id() == "about")
        {
            button->SetText("About FLACHEAD");
        }
    }
}

void SettingsScreen::OnShellUpdate(float deltaSeconds)
{
    (void)deltaSeconds;
    if (m_Volume && Ctx().playback)
    {
        const float current = Ctx().playback->Volume();
        if (std::abs(current - m_Volume->Value()) > 0.001f)
        {
            m_Volume->SetValue(current);
            MarkDirty();
        }
    }
}

bool SettingsScreen::OnShellCommand(flachead::commands::Command command)
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
