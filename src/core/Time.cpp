#include "Time.hpp"

#include <SDL3/SDL.h>

float Time::s_DeltaTime = 0.0f;
float Time::s_FPS = 0.0f;
unsigned long long Time::s_LastCounter = 0;

void Time::Update()
{
    unsigned long long current =
        SDL_GetPerformanceCounter();

    if (s_LastCounter == 0)
    {
        s_LastCounter = current;
        return;
    }

    unsigned long long frequency =
        SDL_GetPerformanceFrequency();

    s_DeltaTime =
        static_cast<float>(current - s_LastCounter) /
        static_cast<float>(frequency);

    s_LastCounter = current;

    if (s_DeltaTime > 0.0f)
        s_FPS = 1.0f / s_DeltaTime;
}

float Time::DeltaTime()
{
    return s_DeltaTime;
}

float Time::FPS()
{
    return s_FPS;
}