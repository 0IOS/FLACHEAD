#include "Application.hpp"

#include <iostream>

Application::Application()
    : m_Running(false)
{
}

Application::~Application()
{
}

bool Application::Initialize()
{
    std::cout << "Initializing FLACHEAD...\n";

    if (!m_window.Create())
        return false;

    m_Running = true;

    return true;
}

void Application::Run()
{
    std::cout << "Running...\n";

    while (m_Running)
    {
        m_Running = false;
    }
}


void Application::Shutdown()
{
    m_window.Destroy();

    std::cout << "Shutting Down...\n";
}