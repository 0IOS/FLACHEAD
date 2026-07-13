#include "Window.hpp"

#include <iostream>

Window::Window()
{
}

Window::~Window()
{
}

bool Window::Create()
{
    std::cout << "Creating Window...\n";

    return true;
}

void Window::Destroy()
{
    std::cout << "Destroying Window...\n";
}