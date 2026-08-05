#pragma once

#include "../commands/Command.hpp"
#include "../input/InputEvent.hpp"
#include "../ui/Canvas.hpp"
#include <SDL3/SDL.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace flachead::screens
{
// Screen lifecycle:
//   OnEnter  — called when the screen becomes the top of the stack.
//   OnExit   — called when the screen is popped or covered.
//   OnUpdate — per-frame update (animations, timers).
//   OnInputEvent — semantic touch/gesture/key events (new shell UI).
//   OnCommand — global commands the screen wants first crack at.
//   HandleEvent — raw SDL events for legacy DAP screens.
class Screen
{
public:
    virtual ~Screen() = default;
    virtual void OnEnter() {}
    virtual void OnExit() {}
    virtual void OnUpdate(float) {}
    virtual void Render(flachead::ui::Canvas&, int, int) {}
    virtual bool HandleEvent(const SDL_Event&) { return false; }
    virtual void OnInputEvent(const flachead::input::InputEvent&) {}
    virtual bool OnCommand(flachead::commands::Command) { return false; }
    virtual bool NeedsRender() const { return false; }
};

class ScreenManager
{
public:
    ScreenManager() = default;
    ~ScreenManager() = default;

    using Factory = std::function<std::unique_ptr<Screen>()>;

    void RegisterFactory(std::string name, Factory factory);
    void Push(std::string_view name);
    void Pop();
    void Clear();
    Screen* Current() const;
    void Update(float deltaSeconds);
    void Render(flachead::ui::Canvas& canvas, int width, int height);
    bool NeedsRender() const;

    bool IsOnStack(std::string_view name) const;
    int Depth() const { return static_cast<int>(m_Stack.size()); }
    std::string_view Top() const { return m_Stack.empty() ? std::string_view{} : m_Stack.back(); }

private:
    std::unordered_map<std::string, Factory> m_Factories;
    std::unordered_map<std::string, std::unique_ptr<Screen>> m_Screens;
    std::vector<std::string> m_Stack;
};
} // namespace flachead::screens
