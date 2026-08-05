#pragma once

#include "../animation/AnimationManager.hpp"
#include "../core/Renderer.hpp"
#include "../dap/AppContext.hpp"
#include "../focus/FocusManager.hpp"
#include "../screens/ScreenManager.hpp"
#include "../ui/overlay/OverlayManager.hpp"
#include "../ui/theme/ThemeManager.hpp"
#include "../ui/wallpaper/WallpaperManager.hpp"

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
};
} // namespace flachead::shell
