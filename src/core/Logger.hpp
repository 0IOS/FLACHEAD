#pragma once

#include <string>
#include <string_view>

namespace flachead::core
{
class Logger
{
public:
    enum class Level
    {
        Info,
        Warning,
        Error,
        Debug
    };

    static void Info(std::string_view message);
    static void Warning(std::string_view message);
    static void Error(std::string_view message);
    static void Debug(std::string_view message);

private:
    static void Write(Level level, std::string_view message);
};
} // namespace flachead::core
