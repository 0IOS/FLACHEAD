#include "Logger.hpp"

#include <iostream>

namespace flachead::core
{
namespace
{
const char* ToString(Logger::Level level)
{
    switch (level)
    {
        case Logger::Level::Info:
            return "INFO";
        case Logger::Level::Warning:
            return "WARN";
        case Logger::Level::Error:
            return "ERROR";
        case Logger::Level::Debug:
            return "DEBUG";
    }
    return "INFO";
}
} // namespace

void Logger::Info(std::string_view message)
{
    Write(Level::Info, message);
}

void Logger::Warning(std::string_view message)
{
    Write(Level::Warning, message);
}

void Logger::Error(std::string_view message)
{
    Write(Level::Error, message);
}

void Logger::Debug(std::string_view message)
{
    Write(Level::Debug, message);
}

void Logger::Write(Level level, std::string_view message)
{
    std::cout << "[FLACHEAD][" << ToString(level) << "] " << message << '\n';
}
} // namespace flachead::core
