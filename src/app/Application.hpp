#pragma once

#include "../system/Window.hpp"

class Application
{
public:
    Application();
    ~Application();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    bool m_Running;
    Window m_Window;
};