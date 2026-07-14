#pragma once

#include "../ui/Canvas.hpp"
#include "../system/Window.hpp"
#include "../core/Renderer.hpp"
#include "../screens/HomeScreen.hpp"

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
    HomeScreen m_HomeScreen;
    Renderer m_Renderer;
    Canvas* m_Canvas;

    bool m_Running;
};