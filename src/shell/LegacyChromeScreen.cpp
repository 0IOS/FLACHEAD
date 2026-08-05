#include "LegacyChromeScreen.hpp"

namespace flachead::shell
{
LegacyChromeScreen::LegacyChromeScreen(std::unique_ptr<flachead::screens::Screen> inner,
                                       const ShellServices& services)
    : m_Inner(std::move(inner)),
      m_Services(services)
{
}

void LegacyChromeScreen::OnEnter()
{
    m_Inner->OnEnter();
}

void LegacyChromeScreen::OnExit()
{
    m_Inner->OnExit();
}

void LegacyChromeScreen::OnUpdate(float deltaSeconds)
{
    m_Inner->OnUpdate(deltaSeconds);
}

void LegacyChromeScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    m_ViewWidth = width;
    m_ViewHeight = height;

    const float navH = m_Chrome.NavHeight(width);
    m_Inner->Render(canvas, width, static_cast<int>(static_cast<float>(height) - navH));
    m_Chrome.DrawNav(canvas, width, height);
}

bool LegacyChromeScreen::HandleEvent(const SDL_Event& event)
{
    return m_Inner->HandleEvent(event);
}

void LegacyChromeScreen::OnInputEvent(const flachead::input::InputEvent& event)
{
    if (m_ViewWidth <= 0 || m_ViewHeight <= 0 ||
        (event.action != flachead::input::InputAction::Tap &&
         event.action != flachead::input::InputAction::Press))
    {
        return;
    }
    const flachead::commands::Command nav =
        m_Chrome.NavCommandAt(m_ViewWidth, m_ViewHeight, event.position);
    if (nav != flachead::commands::Command::None && m_Services.commandCenter)
    {
        m_Services.commandCenter->Dispatch(nav);
    }
}

bool LegacyChromeScreen::OnCommand(flachead::commands::Command command)
{
    return m_Inner->OnCommand(command);
}

bool LegacyChromeScreen::NeedsRender() const
{
    return m_Inner->NeedsRender();
}
} // namespace flachead::shell
