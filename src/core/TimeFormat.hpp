#pragma once

#include <cstdio>
#include <string>

namespace flachead::core
{
namespace timeformat
{
// "3:45" or "1:02:33" for elapsed/remaining time.
inline std::string Duration(double seconds)
{
    if (seconds < 0.0)
    {
        seconds = 0.0;
    }

    const int total = static_cast<int>(seconds + 0.5);
    const int hours = total / 3600;
    const int mins = (total % 3600) / 60;
    const int secs = total % 60;

    char buffer[16];
    if (hours > 0)
    {
        std::snprintf(buffer, sizeof(buffer), "%d:%02d:%02d", hours, mins, secs);
    }
    else
    {
        std::snprintf(buffer, sizeof(buffer), "%d:%02d", mins, secs);
    }
    return buffer;
}

// "3 min" / "2 h 5 min" style short label used by library views.
inline std::string DurationShort(double seconds)
{
    if (seconds <= 0.0)
    {
        return "-";
    }

    const int total = static_cast<int>(seconds + 0.5);
    if (total < 60)
    {
        return std::to_string(total) + "s";
    }

    const int hours = total / 3600;
    const int mins = (total % 3600) / 60;
    if (hours > 0)
    {
        char buffer[16];
        std::snprintf(buffer, sizeof(buffer), "%dh %dm", hours, mins);
        return buffer;
    }

    return std::to_string(mins) + "m";
}
} // namespace timeformat
} // namespace flachead::core
