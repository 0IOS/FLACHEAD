#include "ShellScreen.hpp"

#include "../ui/Button.hpp"

#include <algorithm>

namespace flachead::shell
{
namespace
{
void CollectFocusable(const flachead::ui::Widget& widget, const std::string& prefix,
                      std::unordered_map<std::string, flachead::ui::Widget*>& out)
{
    if (widget.Focusable() && !widget.Id().empty())
    {
        out.emplace(prefix + "." + std::string{widget.Id()}, const_cast<flachead::ui::Widget*>(&widget));
    }
    for (const auto& child : widget.Children())
    {
        CollectFocusable(*child, prefix, out);
    }
}
} // namespace

ShellScreen::ShellScreen(const ShellServices& services)
    : m_Services(services),
      m_Root(std::make_unique<flachead::ui::Container>()),
      m_FocusPrefix("shell")
{
}

void ShellScreen::RebuildTree()
{
    m_Root->RemoveAllChildren();
    m_WidgetsById.clear();
    Focus().Clear();
    BuildShell();
    m_Dirty = true;
}

void ShellScreen::OnEnter()
{
    m_FocusPrefix = std::string{ScreenKey()};
    RebuildTree();
    Wallpaper().SetDim(0.45f);
    Focus().SetChangeCallback([this](std::string_view id) {
        const auto it = m_WidgetsById.find(std::string{id});
        if (it != m_WidgetsById.end())
        {
            it->second->NotifyFocusGained();
        }
    });
    FocusInitial();
    OnShellEnter();
}

void ShellScreen::OnExit()
{
    OnShellExit();
    for (auto& [id, widget] : m_WidgetsById)
    {
        (void)id;
        widget->NotifyFocusLost();
    }
    Focus().SetChangeCallback({});
    Focus().Clear();
    Wallpaper().SetDim(0.0f);
}

void ShellScreen::SyncFocus()
{
    Focus().Clear();
    m_WidgetsById.clear();
    CollectFocusable(*m_Root, m_FocusPrefix, m_WidgetsById);
    for (const auto& [id, widget] : m_WidgetsById)
    {
        Focus().Register(id, widget->GlobalBounds());
    }
    if (!m_FocusedWidgetId.empty() && m_WidgetsById.count(m_FocusedWidgetId) > 0)
    {
        Focus().SetFocus(m_FocusedWidgetId);
    }
    m_Dirty = true;
}

void ShellScreen::FocusInitial()
{
    SyncFocus();
    if (m_WidgetsById.empty())
    {
        m_FocusedWidgetId.clear();
        return;
    }
    const auto& first = *m_WidgetsById.begin();
    m_FocusedWidgetId = first.first;
    Focus().SetFocus(first.first);
    first.second->NotifyFocusGained();
}

void ShellScreen::SetFocusedWidget(std::string_view id)
{
    const std::string full = m_FocusPrefix + "." + std::string{id};
    if (Focus().HasFocus(full))
    {
        m_FocusedWidgetId = full;
        Focus().SetFocus(full);
    }
}

void ShellScreen::OnUpdate(float deltaSeconds)
{
    OnShellUpdate(deltaSeconds);
    m_Root->Update(deltaSeconds);
    Overlays().Update(deltaSeconds);
}

void ShellScreen::Render(flachead::ui::Canvas& canvas, int width, int height)
{
    Wallpaper().Draw(canvas, width, height);

    m_Root->SetBounds(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)});
    m_Root->Layout(Rect{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)});
    SyncFocus();
    m_Root->Draw(canvas);

    if (!Overlays().IsEmpty())
    {
        Overlays().Draw(canvas);
    }
    m_Dirty = false;
}

void ShellScreen::OnInputEvent(const flachead::input::InputEvent& event)
{
    if (!Overlays().IsEmpty())
    {
        if (event.action == flachead::input::InputAction::Tap)
        {
            Overlays().DismissAll();
        }
        return;
    }
    if (HandleWidgetInput(event))
    {
        return;
    }
    OnShellInput(event);
}

bool ShellScreen::HandleWidgetInput(const flachead::input::InputEvent& event)
{
    const bool handled = m_Root->HandleEvent(event);
    if (handled && (event.action == flachead::input::InputAction::Tap ||
                    event.action == flachead::input::InputAction::Press))
    {
        const flachead::ui::Widget* touched = m_Root->ChildAtPoint(event.position);
        if (touched && touched->Focusable() && !touched->Id().empty())
        {
            m_FocusedWidgetId = m_FocusPrefix + "." + std::string{touched->Id()};
            Focus().SetFocus(m_FocusedWidgetId);
        }
    }
    return handled;
}

bool ShellScreen::OnCommand(flachead::commands::Command command)
{
    if (HandleFocusCommand(command))
    {
        return true;
    }
    if (OnShellCommand(command))
    {
        return true;
    }
    return false;
}

bool ShellScreen::HandleFocusCommand(flachead::commands::Command command)
{
    switch (command)
    {
        case flachead::commands::Command::Up:
        case flachead::commands::Command::Down:
        case flachead::commands::Command::Left:
        case flachead::commands::Command::Right:
        {
            const std::string_view previous = Focus().FocusedId();
            if (!Focus().Move(command))
            {
                return true;
            }
            const std::string_view current = Focus().FocusedId();
            m_FocusedWidgetId = std::string{current};
            if (auto it = m_WidgetsById.find(std::string{current}); it != m_WidgetsById.end())
            {
                it->second->NotifyFocusGained();
            }
            if (!previous.empty())
            {
                if (auto it = m_WidgetsById.find(std::string{previous}); it != m_WidgetsById.end())
                {
                    it->second->NotifyFocusLost();
                }
            }
            MarkDirty();
            return true;
        }
        case flachead::commands::Command::Select:
        {
            // The KeyDown InputEvent already activated the focused widget
            // (widgets route it in OnInputEvent before this command arrives);
            // consuming it here prevents double activation.
            return Focus().HasFocusable() && !Focus().FocusedId().empty();
        }
        default:
            return false;
    }
}

bool ShellScreen::NeedsRender() const
{
    return m_Dirty || Animations().ActiveCount() > 0 || !Overlays().IsEmpty() || Wallpaper().NeedsRender();
}
} // namespace flachead::shell
