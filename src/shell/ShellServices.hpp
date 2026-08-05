#pragma once

#include "../animation/AnimationManager.hpp"
#include "../core/Renderer.hpp"
#include "../dap/AppContext.hpp"
#include "../focus/FocusManager.hpp"
#include "../screens/ScreenManager.hpp"
#include "../services/BackgroundJobManager.hpp"
#include "../services/MemoryManager.hpp"
#include "../services/NotificationManager.hpp"
#include "../ui/overlay/OverlayManager.hpp"
#include "../ui/theme/ThemeManager.hpp"
#include "../ui/wallpaper/WallpaperManager.hpp"

#include <functional>

namespace flachead::shell
{
// The services every operating-environment screen needs, bundled like the DAP
// AppContext. Application owns one instance and passes it to every shell
// screen factory; screens read through these pointers and never touch
// Application directly.
struct ShellServices
{
    flachead::dap::AppContext app;
    flachead::core::Renderer* renderer{nullptr};
    flachead::theme::ThemeManager* themes{nullptr};
    flachead::ui::overlay::OverlayManager* overlays{nullptr};
    flachead::ui::wallpaper::WallpaperManager* wallpaper{nullptr};
    flachead::animation::AnimationManager* animations{nullptr};
    flachead::focus::FocusManager* focus{nullptr};
    flachead::screens::ScreenManager* screens{nullptr};
    flachead::services::BackgroundJobManager* backgroundJobs{nullptr};
    flachead::services::NotificationManager* notifications{nullptr};
    flachead::services::MemoryManager* memory{nullptr};

    // Requests a clean application exit (system screen power actions).
    std::function<void()> quit;
};
} // namespace flachead::shell
