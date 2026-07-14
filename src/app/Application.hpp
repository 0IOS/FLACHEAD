#pragma once

#include "../system/Window.hpp"
#include "../core/Renderer.hpp"

class Application
{
public:
    Application();
    ~Application();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    Window m_Window;
    Renderer m_Renderer;

    bool m_Running;
};