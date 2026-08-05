#include "InputManager.hpp"

#include <SDL3/SDL_timer.h>

#include <algorithm>

namespace flachead::input
{
namespace
{
Vec2 TouchPosition(const SDL_Event& event)
{
    return Vec2{event.tfinger.x, event.tfinger.y};
}

Vec2 NormalizedMousePosition(const SDL_Event& event, int windowWidth, int windowHeight)
{
    return Vec2{static_cast<float>(event.button.x) / static_cast<float>(std::max(1, windowWidth)),
                static_cast<float>(event.button.y) / static_cast<float>(std::max(1, windowHeight))};
}
} // namespace

void InputManager::SetWindowSize(int width, int height)
{
    m_WindowWidth = std::max(1, width);
    m_WindowHeight = std::max(1, height);
}

InputManager::InputManager(GestureConfig config)
    : m_Config(config),
      m_Gestures(config, [this](const Gesture& gesture) {
          InputEvent event;
          event.source = m_PointerSource;
          event.position = gesture.position;
          event.delta = gesture.delta;
          event.velocity = gesture.velocity;
          event.tapCount = gesture.tapCount;
          switch (gesture.type)
          {
              case GestureType::Tap: event.action = InputAction::Tap; break;
              case GestureType::DoubleTap: event.action = InputAction::DoubleTap; break;
              case GestureType::Hold: event.action = InputAction::Hold; break;
              case GestureType::Swipe: event.action = InputAction::Swipe; break;
              case GestureType::DragBegin: event.action = InputAction::Press; break;
              case GestureType::DragMove: event.action = InputAction::DragMove; break;
              case GestureType::DragEnd: event.action = InputAction::DragEnd; break;
          }
          EmitInput(event);
      })
{
}

void InputManager::Initialize()
{
    m_Gestures.Cancel();
    m_HomeDown = false;
    m_HomeTapCount = 0;
    m_HomePending = false;
    m_LastHomeReleaseValid = false;
}

void InputManager::Shutdown()
{
    m_Gestures.Cancel();
    m_TouchDown = false;
    m_HomeDown = false;
    m_HomePending = false;
    m_LastHomeReleaseValid = false;
}

void InputManager::EmitInput(const InputEvent& event)
{
    if (m_InputEventCallback)
    {
        m_InputEventCallback(event);
    }
}

void InputManager::EmitCommand(commands::Command command)
{
    if (m_CommandCallback)
    {
        m_CommandCallback(command);
    }
}

void InputManager::HandlePointerDown(const Vec2& position)
{
    const Vec2 pixels{position.x * static_cast<float>(m_WindowWidth),
                       position.y * static_cast<float>(m_WindowHeight)};
    m_TouchDown = true;
    m_Gestures.OnPress(pixels, SDL_GetTicks());
    InputEvent event;
    event.action = InputAction::Press;
    event.source = m_PointerSource;
    event.position = pixels;
    EmitInput(event);
}

void InputManager::HandlePointerMove(const Vec2& position)
{
    if (!m_TouchDown)
    {
        return;
    }
    m_Gestures.OnMove(Vec2{position.x * static_cast<float>(m_WindowWidth),
                           position.y * static_cast<float>(m_WindowHeight)},
                      SDL_GetTicks());
}

void InputManager::HandlePointerUp(const Vec2& position)
{
    if (!m_TouchDown)
    {
        return;
    }
    const Vec2 pixels{position.x * static_cast<float>(m_WindowWidth),
                      position.y * static_cast<float>(m_WindowHeight)};
    m_TouchDown = false;
    m_Gestures.OnRelease(pixels, SDL_GetTicks());
    InputEvent event;
    event.action = InputAction::Release;
    event.source = m_PointerSource;
    event.position = pixels;
    EmitInput(event);
}

void InputManager::HandleHomeKey(bool down)
{
    const uint64_t nowMs = SDL_GetTicks();
    if (down)
    {
        if (!m_HomeDown)
        {
            m_HomeDown = true;
            m_HomeDownTimeMs = nowMs;
            m_HomeHoldEmitted = false;
        }
        if (m_HomeDown && !m_HomeHoldEmitted && nowMs - m_HomeDownTimeMs >= m_Config.holdDelayMs)
        {
            m_HomeHoldEmitted = true;
            m_HomeTapCount = 0;
            m_HomePending = false;
            m_LastHomeReleaseValid = false;
            EmitCommand(commands::Command::TaskOverview);
        }
        return;
    }

    if (!m_HomeDown)
    {
        return;
    }
    m_HomeDown = false;
    if (m_HomeHoldEmitted)
    {
        m_HomeHoldEmitted = false;
        m_LastHomeReleaseValid = false;
        return;
    }

    const uint64_t heldMs = nowMs - m_HomeDownTimeMs;
    if (heldMs >= m_Config.holdDelayMs)
    {
        m_LastHomeReleaseValid = false;
        return;
    }

    if (m_LastHomeReleaseValid && nowMs - m_LastHomeReleaseMs <= m_Config.doubleTapWindowMs)
    {
        m_HomePending = false;
        m_HomeTapCount = 0;
        m_LastHomeReleaseValid = true;
        EmitCommand(commands::Command::Home);
        m_LastHomeReleaseMs = nowMs;
        return;
    }

    m_HomeTapCount = 1;
    m_HomePending = true;
    m_HomePendingSinceMs = nowMs;
    m_LastHomeReleaseMs = nowMs;
    m_LastHomeReleaseValid = true;

    // Announce the pending single-tap action so the shell can hint that a
    // second tap within the window would go Home. The real Launcher command
    // fires when the window expires (see Update()).
    InputEvent pendingEvent;
    pendingEvent.action = InputAction::Tap;
    pendingEvent.source = InputSource::Gpio;
    pendingEvent.command = commands::Command::Launcher;
    pendingEvent.position = Vec2{-1.0f, -1.0f};
    EmitInput(pendingEvent);
}

void InputManager::Update()
{
    m_Gestures.Update(SDL_GetTicks());
    if (!m_HomePending)
    {
        return;
    }
    if (SDL_GetTicks() - m_HomePendingSinceMs >= m_Config.doubleTapWindowMs)
    {
        m_HomePending = false;
        if (m_HomeTapCount == 1)
        {
            EmitCommand(commands::Command::Launcher);
        }
        m_HomeTapCount = 0;
    }
}

void InputManager::HandleEvent(const SDL_Event& event)
{
    switch (event.type)
    {
        case SDL_EVENT_FINGER_DOWN:
            m_PointerSource = InputSource::Touch;
            HandlePointerDown(TouchPosition(event));
            break;
        case SDL_EVENT_FINGER_MOTION:
            HandlePointerMove(TouchPosition(event));
            break;
        case SDL_EVENT_FINGER_UP:
            HandlePointerUp(TouchPosition(event));
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                m_PointerSource = InputSource::Mouse;
                HandlePointerDown(NormalizedMousePosition(event, m_WindowWidth, m_WindowHeight));
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                HandlePointerUp(NormalizedMousePosition(event, m_WindowWidth, m_WindowHeight));
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (m_TouchDown)
            {
                HandlePointerMove(NormalizedMousePosition(event, m_WindowWidth, m_WindowHeight));
            }
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_HOME || event.key.key == SDLK_AC_HOME)
            {
                HandleHomeKey(true);
                return;
            }
            {
                InputEvent inputEvent;
                inputEvent.action = InputAction::KeyDown;
                inputEvent.source = InputSource::Keyboard;
                inputEvent.key = event.key.key;
                EmitInput(inputEvent);
                EmitCommand(KeyToCommand(event.key.key));
            }
            break;
        case SDL_EVENT_KEY_UP:
            if (event.key.key == SDLK_HOME || event.key.key == SDLK_AC_HOME)
            {
                HandleHomeKey(false);
                return;
            }
            {
                InputEvent inputEvent;
                inputEvent.action = InputAction::KeyUp;
                inputEvent.source = InputSource::Keyboard;
                inputEvent.key = event.key.key;
                EmitInput(inputEvent);
            }
            break;
        default:
            break;
    }
}
} // namespace flachead::input
