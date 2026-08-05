#pragma once

#include "../events/EventBus.hpp"
#include "../library/LibraryService.hpp"
#include "../playback/PlaybackController.hpp"
#include "../playback/PlaylistEngine.hpp"
#include "../services/SettingsManager.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace flachead::dap
{
// Service bundle handed to every DAP screen. Screens read and drive playback /
// library / settings exclusively through these pointers; they never touch the
// Database or EventBus internals directly.
struct AppContext
{
    flachead::events::EventBus* eventBus{nullptr};
    flachead::library::LibraryService* library{nullptr};
    flachead::playback::PlaybackController* playback{nullptr};
    flachead::services::SettingsManager* settings{nullptr};
    flachead::playback::PlaylistEngine* playlists{nullptr};

    std::vector<std::string> scanRoots;

    // Destination parameters for the next screen. A source screen fills these
    // before navigating; the target reads them in RefreshData.
    struct Pending
    {
        std::string artist;
        std::string album;
        std::string albumArtist;
        std::int64_t playlistId{0};
    };
    Pending pending;

    // Screen navigation (factory names registered by Application).
    std::function<void(std::string_view)> navigate;
    std::function<void()> goBack;
};
} // namespace flachead::dap
