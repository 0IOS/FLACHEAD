#pragma once

#include <SDL3/SDL.h>

#include <functional>

namespace flachead::system
{
struct WindowSize
{
    int width;
    int height;
};

class Window
{
public:
    Window();
    ~Window();

    bool Create();
    void Destroy();

    bool PollEvents(const std::function<bool(const SDL_Event&)>& handler = {});

    void WaitForEvent(int timeoutMs);

    SDL_Window* GetNativeWindow() const;
    WindowSize GetSize() const;

private:
    SDL_Window* m_Window{nullptr};
};
} // namespace flachead::system