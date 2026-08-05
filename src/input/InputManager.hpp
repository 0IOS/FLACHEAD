#pragma once

#include "GestureRecognizer.hpp"
#include "InputEvent.hpp"

#include <SDL3/SDL_events.h>

#include <cstdint>
#include <functional>

namespace flachead::input
{
// The single input facade for the operating environment. The application
// feeds every raw SDL event in; the manager:
//   - routes pointer events through the GestureRecognizer,
//   - maps keys to commands (KeyToCommand),
//   - implements the center/home button semantics (tap -> launcher,
//     double tap -> home, hold -> task overview).
// The current screen receives both the raw SDL event (for legacy DAP screens)
// and the semantic InputEvents / Commands (for the new shell and widgets).
class InputManager
{
public:
    using InputEventCallback = std::function<void(const InputEvent&)>;
    using CommandCallback = std::function<void(commands::Command)>;

    explicit InputManager(GestureConfig config = {});

    void Initialize();
    void Shutdown();

    // The logical window size. Touch coordinates arrive normalized (0..1) and
    // are converted to pixels using this.
    void SetWindowSize(int width, int height);

    void SetInputEventCallback(InputEventCallback callback) { m_InputEventCallback = std::move(callback); }
    void SetCommandCallback(CommandCallback callback) { m_CommandCallback = std::move(callback); }

    // Process one raw SDL event. Call once per event from the input backend.
    void HandleEvent(const SDL_Event& event);

    // Fire any pending timed commands (single home-tap disambiguation) and
    // drive the gesture recognizer's hold timer.
    void Update();

    bool IsTouchActive() const { return m_TouchDown; }

private:
    void EmitInput(const InputEvent& event);
    void EmitCommand(commands::Command command);
    void HandlePointerDown(const Vec2& position);
    void HandlePointerMove(const Vec2& position);
    void HandlePointerUp(const Vec2& position);
    void HandleHomeKey(bool down);

    GestureConfig m_Config;
    GestureRecognizer m_Gestures;
    InputEventCallback m_InputEventCallback;
    CommandCallback m_CommandCallback;

    bool m_TouchDown{false};
    InputSource m_PointerSource{InputSource::Touch};
    int m_WindowWidth{320};
    int m_WindowHeight{240};

    bool m_HomeDown{false};
    uint64_t m_HomeDownTimeMs{0};
    uint64_t m_LastHomeReleaseMs{0};
    bool m_LastHomeReleaseValid{false};
    int m_HomeTapCount{0};
    bool m_HomeHoldEmitted{false};
    bool m_HomePending{false};
    uint64_t m_HomePendingSinceMs{0};
};
} // namespace flachead::input
