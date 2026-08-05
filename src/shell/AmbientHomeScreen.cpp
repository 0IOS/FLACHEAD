#include "AmbientHomeScreen.hpp"

#include "../models/SongModel.hpp"
#include "../ui/Button.hpp"
#include "../ui/Image.hpp"
#include "../ui/Label.hpp"
#include "../ui/ProgressBar.hpp"

#include <algorithm>
#include <cstdio>

namespace flachead::shell
{
namespace
{
flachead::ui::Button* MakeButton(const std::string& text, std::string id,
                                 flachead::ui::Widget& parent, flachead::ui::Button::ClickHandler onTap)
{
    auto button = std::make_unique<flachead::ui::Button>();
    button->SetId(id);
    button->SetText(text);
    button->SetClickHandler(std::move(onTap));
    button->SetFocusable(true);
    flachead::ui::Button* raw = button.get();
    parent.AddChild(std::move(button));
    return raw;
}

flachead::ui::Label* MakeLabel(const std::string& text, float size, flachead::ui::Widget& parent)
{
    auto label = std::make_unique<flachead::ui::Label>();
    label->SetText(text);
    label->SetFontSize(size);
    label->SetAlign(flachead::ui::Label::Align::Center);
    flachead::ui::Label* raw = label.get();
    parent.AddChild(std::move(label));
    return raw;
}
} // namespace

AmbientHomeScreen::AmbientHomeScreen(const ShellServices& services)
    : ShellScreen(services)
{
}

std::string AmbientHomeScreen::FormatTime(double seconds) const
{
    if (seconds < 0.0)
    {
        seconds = 0.0;
    }
    const int total = static_cast<int>(seconds);
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d:%02d", total / 60, total % 60);
    return buffer;
}

void AmbientHomeScreen::BuildShell()
{
    Root().RemoveAllChildren();
    const bool hasTrack = Ctx().playback->CurrentTrack().Valid();
    m_Idle = !hasTrack;
    if (hasTrack)
    {
        BuildPlayerView();
    }
    else
    {
        BuildIdleView();
    }
    m_LastTrackId = 0;
    m_LastArtPath.clear();
    m_HasArt = false;
}

void AmbientHomeScreen::BuildPlayerView()
{
    auto& root = Root();
    const flachead::palette::Palette& p = Themes().ActivePalette();

    auto panel = std::make_unique<flachead::ui::Container>();
    panel->SetId("panel");
    panel->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Vertical));
    flachead::ui::Widget* panelRaw = panel.get();
    root.AddChild(std::move(panel));

    m_Cover = new flachead::ui::Image();
    m_Cover->SetId("cover");
    auto cover = std::unique_ptr<flachead::ui::Image>(m_Cover);
    panelRaw->AddChild(std::move(cover));

    m_Title = MakeLabel("", 16.0f, *panelRaw);
    m_Subtitle = MakeLabel("", 12.0f, *panelRaw);

    m_Progress = new flachead::ui::ProgressBar();
    m_Progress->SetColor(p.accent);
    m_Progress->SetTrackColor(p.border);
    m_Progress->SetCornerRadius(3.0f);
    m_Progress->SetFocusable(false);
    auto progress = std::unique_ptr<flachead::ui::ProgressBar>(m_Progress);
    panelRaw->AddChild(std::move(progress));

    auto timeRow = std::make_unique<flachead::ui::Container>();
    timeRow->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Horizontal));
    flachead::ui::Widget* timeRowRaw = timeRow.get();
    panelRaw->AddChild(std::move(timeRow));
    m_Position = MakeLabel("0:00", 10.0f, *timeRowRaw);
    m_Position->SetAlign(flachead::ui::Label::Align::Left);
    m_Duration = MakeLabel("0:00", 10.0f, *timeRowRaw);
    m_Duration->SetAlign(flachead::ui::Label::Align::Right);

    auto transport = std::make_unique<flachead::ui::Container>();
    transport->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Horizontal));
    flachead::ui::Widget* transportRaw = transport.get();
    panelRaw->AddChild(std::move(transport));

    auto prev = MakeButton("|<", "prev", *transportRaw, [this] { Ctx().playback->Previous(); });
    (void)prev;
    m_PlayPause = MakeButton(">", "playpause", *transportRaw, [this] { Ctx().playback->Toggle(); });
    auto next = MakeButton(">|", "next", *transportRaw, [this] { Ctx().playback->Next(); });
    (void)next;

    auto utility = std::make_unique<flachead::ui::Container>();
    utility->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Horizontal));
    flachead::ui::Widget* utilityRaw = utility.get();
    panelRaw->AddChild(std::move(utility));

    m_Favorite = MakeButton("Fav", "fav", *utilityRaw, [this] {
        const auto& track = Ctx().playback->CurrentTrack();
        if (track.Valid() && Ctx().library)
        {
            Ctx().library->ToggleFavorite(track.id);
            MarkDirty();
        }
    });
    m_Shuffle = MakeButton("Shuf", "shuffle", *utilityRaw, [this] { Ctx().playback->ToggleShuffle(); });
    m_Repeat = MakeButton("Rep", "repeat", *utilityRaw, [this] { Ctx().playback->ToggleRepeat(); });
    auto search = MakeButton("Find", "search", *utilityRaw, [this] {
        Ctx().navigate("universal_search");
    });
    (void)search;
    auto menu = MakeButton("Apps", "menu", *utilityRaw, [this] { Ctx().navigate("launcher"); });
    (void)menu;

    RefreshTrack();
}

void AmbientHomeScreen::BuildIdleView()
{
    auto& root = Root();
    auto panel = std::make_unique<flachead::ui::Container>();
    panel->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Vertical));
    flachead::ui::Widget* panelRaw = panel.get();
    root.AddChild(std::move(panel));

    m_EmptyTitle = MakeLabel("Nothing playing", 18.0f, *panelRaw);
    auto hint = MakeLabel("Tap Apps to browse your library", 12.0f, *panelRaw);
    (void)hint;

    auto row = std::make_unique<flachead::ui::Container>();
    row->SetLayout(flachead::layout::MakeBox(flachead::layout::Orientation::Horizontal));
    flachead::ui::Widget* rowRaw = row.get();
    panelRaw->AddChild(std::move(row));

    auto apps = MakeButton("Apps", "apps", *rowRaw, [this] { Ctx().navigate("launcher"); });
    (void)apps;
    auto find = MakeButton("Find", "find", *rowRaw, [this] { Ctx().navigate("universal_search"); });
    (void)find;
    auto settings = MakeButton("Settings", "settings", *rowRaw, [this] { Ctx().navigate("dapsettings"); });
    (void)settings;
}

void AmbientHomeScreen::OnShellEnter()
{
    RefreshTrack();
    RefreshArt();
}

void AmbientHomeScreen::RefreshTrack()
{
    const flachead::models::SongModel& track = Ctx().playback->CurrentTrack();
    const bool hasTrack = track.Valid();

    if (hasTrack != m_Idle)
    {
        m_Idle = !hasTrack;
        RebuildTree();
        return;
    }
    if (!hasTrack)
    {
        return;
    }

    bool changed = false;

    const std::string title = track.DisplayTitle();
    const std::string subtitle = track.DisplayArtist() + "  -  " + track.DisplayAlbum();
    if (title != m_LastTitle)
    {
        m_LastTitle = title;
        changed = true;
    }
    if (subtitle != m_LastSubtitle)
    {
        m_LastSubtitle = subtitle;
        changed = true;
    }
    if (m_Title)
    {
        m_Title->SetText(m_LastTitle);
    }
    if (m_Subtitle)
    {
        m_Subtitle->SetText(m_LastSubtitle);
    }

    if (m_Progress)
    {
        const double duration = Ctx().playback->DurationSeconds();
        const double position = Ctx().playback->PositionSeconds();
        const float progress = duration > 0.0 ? static_cast<float>(std::clamp(position / duration, 0.0, 1.0)) : 0.0f;
        if (std::abs(progress - m_LastProgress) > 0.0005f)
        {
            m_LastProgress = progress;
            m_Progress->SetValue(progress);
            changed = true;
        }
        const std::string pos = FormatTime(position);
        const std::string dur = FormatTime(duration);
        if (pos != m_LastPosition)
        {
            m_LastPosition = pos;
            if (m_Position)
            {
                m_Position->SetText(pos);
            }
            changed = true;
        }
        if (dur != m_LastDuration)
        {
            m_LastDuration = dur;
            if (m_Duration)
            {
                m_Duration->SetText(dur);
            }
            changed = true;
        }
    }

    const bool playing = Ctx().playback->IsPlaying();
    if (playing != m_LastPlaying)
    {
        m_LastPlaying = playing;
        if (m_PlayPause)
        {
            m_PlayPause->SetText(playing ? "II" : ">");
        }
        changed = true;
    }
    const bool favorite = track.favorite;
    if (favorite != m_LastFavorite)
    {
        m_LastFavorite = favorite;
        if (m_Favorite)
        {
            m_Favorite->SetText(favorite ? "*Fav" : "Fav");
        }
        changed = true;
    }

    if (track.id != m_LastTrackId)
    {
        m_LastTrackId = track.id;
        m_LastArtPath.clear();
        m_HasArt = false;
        RefreshArt();
        changed = true;
    }

    if (changed)
    {
        MarkDirty();
    }
}

void AmbientHomeScreen::RefreshArt()
{
    const flachead::models::SongModel& track = Ctx().playback->CurrentTrack();
    const std::string& path = track.artPath;
    if (path.empty() || path == m_LastArtPath)
    {
        return;
    }
    m_LastArtPath = path;

    int width = 0;
    int height = 0;
    auto texture = Renderer().LoadTexture(path, &width, &height);
    if (!texture)
    {
        if (m_Cover)
        {
            m_Cover->SetTexture(nullptr);
        }
        m_HasArt = false;
        Wallpaper().ClearTexture();
        return;
    }

    if (m_Cover)
    {
        m_Cover->SetTexture(texture);
    }
    Wallpaper().SetTexture(texture, width, height);
    m_HasArt = true;
    Animations().Cancel(m_ArtAnimation);
    m_ArtAnimation = Animations().AnimateFloat(0.0f, 1.0f, 0.4f, flachead::animation::Easing::EaseOutCubic,
                                               [this](float value) { Wallpaper().SetCrossfade(value); });
    MarkDirty();
}

void AmbientHomeScreen::OnShellUpdate(float deltaSeconds)
{
    (void)deltaSeconds;
    RefreshTrack();
}

bool AmbientHomeScreen::OnShellCommand(flachead::commands::Command command)
{
    switch (command)
    {
        case flachead::commands::Command::PlayPause:
            Ctx().playback->Toggle();
            return true;
        case flachead::commands::Command::Next:
            Ctx().playback->Next();
            return true;
        case flachead::commands::Command::Previous:
            Ctx().playback->Previous();
            return true;
        case flachead::commands::Command::ToggleFavorite:
            if (Ctx().library && Ctx().playback->CurrentTrack().Valid())
            {
                Ctx().library->ToggleFavorite(Ctx().playback->CurrentTrack().id);
            }
            return true;
        case flachead::commands::Command::VolumeUp:
            Ctx().playback->SetVolume(std::min(1.0f, Ctx().playback->Volume() + 0.05f));
            return true;
        case flachead::commands::Command::VolumeDown:
            Ctx().playback->SetVolume(std::max(0.0f, Ctx().playback->Volume() - 0.05f));
            return true;
        default:
            return false;
    }
}

void AmbientHomeScreen::OnShellInput(const flachead::input::InputEvent& event)
{
    if (event.action == flachead::input::InputAction::DoubleTap)
    {
        Ctx().navigate("launcher");
    }
}
} // namespace flachead::shell
