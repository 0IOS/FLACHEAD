#include "app/Application.hpp"
#include "core/Logger.hpp"

#include <chrono>
#include <cstring>
#include <string_view>

int main(int argc, char** argv)
{
    float benchmarkSeconds = 0.0f;

    for (int i = 1; i < argc; ++i)
    {
        const std::string_view arg{argv[i]};
        if (arg == "--benchmark")
        {
            benchmarkSeconds = 5.0f;
        }
        else if (arg.starts_with("--benchmark="))
        {
            benchmarkSeconds = std::stof(std::string{arg.substr(12)});
        }
    }

    const auto startTime = std::chrono::steady_clock::now();

    Application app(benchmarkSeconds);

    if (!app.Initialize())
    {
        return -1;
    }

    const auto readyTime = std::chrono::steady_clock::now();
    const double startupMs = std::chrono::duration<double, std::milli>(readyTime - startTime).count();
    flachead::core::Logger::Info("Startup time: " + std::to_string(startupMs) + " ms");

    app.Run();

    return 0;
}
