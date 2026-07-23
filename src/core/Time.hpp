#pragma once

class Time
{
public:
    static void Update();

    static float DeltaTime();

    static float FPS();

private:
    static float s_DeltaTime;
    static float s_FPS;
    static unsigned long long s_LastCounter;
};