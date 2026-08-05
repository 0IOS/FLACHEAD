#pragma once

#include "../dap/AppContext.hpp"
#include "../dap/DapScreen.hpp"

#include <functional>
#include <string>
#include <vector>

namespace flachead::dap
{
// DAP home: a now-playing hero card plus navigation rows for every library
// screen. ESC asks the host to quit.
class HomeScreen : public DapScreen
{
public:
    explicit HomeScreen(const AppContext& context);

    void RefreshData() override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;

    void SetBackHandler(std::function<void()> handler) { m_OnBack = std::move(handler); }

private:
    void Activate();

    std::function<void()> m_OnBack;
    std::string m_NowPlayingTitle;
    std::string m_NowPlayingArtist;
    bool m_HasPlayback{false};
    bool m_Playing{false};

    struct NavEntry
    {
        std::string name;
        std::string icon;
        std::string screen;
    };

    std::vector<NavEntry> m_Entries{
        {"Now Playing", "\xe2\x96\xb6", "nowplaying"},
        {"Library",     "\xe2\x99\xab", "songs"},
        {"Albums",      "\xe2\x97\x87", "albums"},
        {"Artists",     "\xe2\x99\x80", "artists"},
        {"Favorites",   "\xe2\x99\xa5", "favorites"},
        {"Recently Added", "\xe2\x8c\x9b", "recent"},
        {"Playlists",   "\xe2\x98\xb0", "playlists"},
        {"Search",      "\xe2\x9c\xa6", "search"},
        {"Settings",    "\xe2\x9a\x99", "dapsettings"},
        {"Scan",        "\xe2\x8c\x95", "scan"},
    };

    int m_SelectedIndex{0};
};
} // namespace flachead::dap
