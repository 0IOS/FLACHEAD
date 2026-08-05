#pragma once

#include "../animation/AnimationManager.hpp"
#include "../focus/FocusManager.hpp"
#include "../screens/ScreenManager.hpp"
#include "../ui/Canvas.hpp"
#include "../ui/Container.hpp"
#include "../ui/overlay/OverlayManager.hpp"
#include "../ui/theme/ThemeManager.hpp"
#include "../ui/wallpaper/WallpaperManager.hpp"
#include "ShellChrome.hpp"
#include "ShellServices.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace flachead::shell
{
// Base class of the operating-environment screens. Hosts a widget tree, a
// per-screen focus scope, the wallpaper and the overlay layer. Subclasses
// build their UI in BuildShell and react to commands/input in the OnShell*
// hooks. Rendering order is wallpaper -> widget tree -> overlays.
class ShellScreen : public flachead::screens::Screen
{
public:
    explicit ShellScreen(const ShellServices& services);

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    void OnInputEvent(const flachead::input::InputEvent& event) override;
    bool OnCommand(flachead::commands::Command command) override;
    bool NeedsRender() const override;

    // Markers the application uses to route shell-level commands.
    bool IsShellScreen() const { return true; }
    bool IsChromeScreen() const override { return true; }

    // Title shown in the persistent top status strip.
    virtual std::string_view ChromeTitle() const { return ScreenKey(); }

protected:
    const flachead::dap::AppContext& Ctx() const { return m_Services.app; }
    flachead::dap::AppContext& Ctx() { return m_Services.app; }
    const ShellServices& Services() const { return m_Services; }
    flachead::core::Renderer& Renderer() { return *m_Services.renderer; }
    flachead::theme::ThemeManager& Themes() { return *m_Services.themes; }
    const flachead::theme::ThemeManager& Themes() const { return *m_Services.themes; }
    flachead::ui::overlay::OverlayManager& Overlays() { return *m_Services.overlays; }
    const flachead::ui::overlay::OverlayManager& Overlays() const { return *m_Services.overlays; }
    flachead::ui::wallpaper::WallpaperManager& Wallpaper() { return *m_Services.wallpaper; }
    const flachead::ui::wallpaper::WallpaperManager& Wallpaper() const { return *m_Services.wallpaper; }
    flachead::animation::AnimationManager& Animations() { return *m_Services.animations; }
    const flachead::animation::AnimationManager& Animations() const { return *m_Services.animations; }
    flachead::focus::FocusManager& Focus() { return *m_Services.focus; }
    const flachead::focus::FocusManager& Focus() const { return *m_Services.focus; }
    flachead::ui::Widget& Root() { return *m_Root; }

    // Subclass hooks.
    virtual void BuildShell() = 0;
    virtual void OnShellEnter() {}
    virtual void OnShellExit() {}
    virtual void OnShellUpdate(float deltaSeconds) { (void)deltaSeconds; }
    virtual bool OnShellCommand(flachead::commands::Command command) { (void)command; return false; }
    virtual void OnShellInput(const flachead::input::InputEvent& event) { (void)event; }
    virtual std::string_view ScreenKey() const = 0;

    void MarkDirty() { m_Dirty = true; }
    void RebuildTree();

    // Moves focus to the first registered focusable, or back to the saved one.
    void FocusInitial();
    void SetFocusedWidget(std::string_view id);

    // Handles synthetic command-carrying input (e.g. a pending home tap) that
    // must not be dispatched to widgets.
    void HandleCommandSignal(const flachead::input::InputEvent& event);

    const std::unordered_map<std::string, flachead::ui::Widget*>& WidgetsById() const { return m_WidgetsById; }

private:
    void SyncFocus();
    bool HandleWidgetInput(const flachead::input::InputEvent& event);
    bool HandleFocusCommand(flachead::commands::Command command);

    ShellServices m_Services;
    flachead::shell::ShellChrome m_Chrome{m_Services};
    std::unique_ptr<flachead::ui::Container> m_Root;
    std::unordered_map<std::string, flachead::ui::Widget*> m_WidgetsById;
    std::string m_FocusPrefix;
    std::string m_FocusedWidgetId;
    int m_ViewWidth{0};
    int m_ViewHeight{0};
    bool m_Dirty{true};
};
} // namespace flachead::shell
