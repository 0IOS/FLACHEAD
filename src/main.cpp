#include "app/Application.hpp"

int main()
{
    Application app;

    if (!app.Initialize())
        return -1;

    app.Run();

    return 0;
}