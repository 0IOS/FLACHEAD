#pragma once

#include "ShellScreen.hpp"

#include <cstdint>
#include <string>

namespace flachead::ui
{
class Button;
class Image;
class Label;
class ProgressBar;
} // namespace flachead::ui

namespace flachead::shell
{
// The ambient home: a full-player home screen. Shows the current track with
// cover art (also used as wallpaper), transport controls and utility actions.
// This is the bottom of the stack and what the home button returns to.
class AmbientHomeScreen : public ShellScreen
{
public:
    explicit AmbientHomeScreen(const ShellServices& services);

protected:
    void BuildShell() override;
    void OnShellEnter() override;
    void OnShellUpdate(float deltaSeconds) override;
    bool OnShellCommand(flachead::commands::Command command) override;
    void OnShellInput(const flachead::input::InputEvent& event) override;
    std::string_view ScreenKey() const override { return "home"; }

private:
    void BuildPlayerView();
    void BuildIdleView();
    void RefreshTrack();
    void RefreshArt();
    std::string FormatTime(double seconds) const;

    flachead::ui::Image* m_Cover{nullptr};
    flachead::ui::Label* m_Title{nullptr};
    flachead::ui::Label* m_Subtitle{nullptr};
    flachead::ui::Label* m_Position{nullptr};
    flachead::ui::Label* m_Duration{nullptr};
    flachead::ui::Label* m_EmptyTitle{nullptr};
    flachead::ui::ProgressBar* m_Progress{nullptr};
    flachead::ui::Button* m_PlayPause{nullptr};
    flachead::ui::Button* m_Favorite{nullptr};
    flachead::ui::Button* m_Shuffle{nullptr};
    flachead::ui::Button* m_Repeat{nullptr};

    std::string m_LastArtPath;
    std::int64_t m_LastTrackId{0};
    std::uint64_t m_ArtAnimation{0};
    std::string m_LastTitle;
    std::string m_LastSubtitle;
    float m_LastProgress{-1.0f};
    std::string m_LastPosition;
    std::string m_LastDuration;
    bool m_LastPlaying{false};
    bool m_LastFavorite{false};
    bool m_Idle{false};
    bool m_HasArt{false};
};
} // namespace flachead::shell
