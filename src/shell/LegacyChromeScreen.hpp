#pragma once

#include "../screens/ScreenManager.hpp"
#include "ShellChrome.hpp"
#include "ShellServices.hpp"

#include <memory>

namespace flachead::shell
{
// Wraps a legacy DAP screen in the operating environment's persistent chrome.
// The inner screen keeps its own status/header/footer as the primary content
// region (reduced in height), and the shared bottom navigation bar is drawn
// beneath it. Raw SDL events and lifecycle calls are forwarded to the inner
// screen; semantic taps on the navigation bar are intercepted here.
class LegacyChromeScreen : public flachead::screens::Screen
{
public:
    LegacyChromeScreen(std::unique_ptr<flachead::screens::Screen> inner,
                       const ShellServices& services);

    void OnEnter() override;
    void OnExit() override;
    void OnUpdate(float deltaSeconds) override;
    void Render(flachead::ui::Canvas& canvas, int width, int height) override;
    bool HandleEvent(const SDL_Event& event) override;
    void OnInputEvent(const flachead::input::InputEvent& event) override;
    bool OnCommand(flachead::commands::Command command) override;
    bool NeedsRender() const override;
    bool IsChromeScreen() const override { return true; }

    flachead::screens::Screen* Inner() { return m_Inner.get(); }

private:
    std::unique_ptr<flachead::screens::Screen> m_Inner;
    ShellServices m_Services;
    flachead::shell::ShellChrome m_Chrome{m_Services};
    int m_ViewWidth{0};
    int m_ViewHeight{0};
};
} // namespace flachead::shell
