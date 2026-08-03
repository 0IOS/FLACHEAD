#include "GpioInputBackend.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace flachead::input
{
namespace
{
constexpr const char* kGpioBase = "/sys/class/gpio";
constexpr const char* kExportPath = "/sys/class/gpio/export";
constexpr const char* kUnexportPath = "/sys/class/gpio/unexport";

bool WriteFile(const char* path, const char* value)
{
    FILE* file = std::fopen(path, "w");
    if (!file)
    {
        return false;
    }
    const bool ok = std::fprintf(file, "%s", value) >= 0;
    std::fclose(file);
    return ok;
}
} // namespace

std::vector<GpioInputBackend::Pin> GpioInputBackend::DefaultPins()
{
    return {
        {17, SDLK_UP},
        {27, SDLK_DOWN},
        {22, SDLK_LEFT},
        {23, SDLK_RIGHT},
        {24, SDLK_RETURN},
        {25, SDLK_ESCAPE},
        {5,  SDLK_MEDIA_PLAY_PAUSE},
        {6,  SDLK_MEDIA_NEXT_TRACK},
        {13, SDLK_MEDIA_PREVIOUS_TRACK},
    };
}

GpioInputBackend::GpioInputBackend(std::vector<Pin> pins)
{
    for (const Pin& pin : pins)
    {
        if (m_PinCount >= m_Pins.size())
        {
            break;
        }
        m_Pins[m_PinCount] = PinState{pin.gpio, pin.key, -1, false};
        ++m_PinCount;
    }
}

bool GpioInputBackend::ExportGpio(int gpio)
{
    char number[8];
    std::snprintf(number, sizeof(number), "%d", gpio);
    if (!WriteFile(kExportPath, number))
    {
        return errno == EEXIST || errno == EBUSY;
    }

    char dir[64];
    std::snprintf(dir, sizeof(dir), "%s/gpio%d/direction", kGpioBase, gpio);
    return WriteFile(dir, "in");
}

bool GpioInputBackend::UnexportGpio(int gpio)
{
    char number[8];
    std::snprintf(number, sizeof(number), "%d", gpio);
    return WriteFile(kUnexportPath, number);
}

int GpioInputBackend::OpenValueFile(int gpio)
{
    char path[64];
    std::snprintf(path, sizeof(path), "%s/gpio%d/value", kGpioBase, gpio);
    return ::open(path, O_RDONLY);
}

bool GpioInputBackend::Initialize()
{
    for (size_t i = 0; i < m_PinCount; ++i)
    {
        if (!ExportGpio(m_Pins[i].gpio))
        {
            Shutdown();
            return false;
        }
        m_Pins[i].fd = OpenValueFile(m_Pins[i].gpio);
        if (m_Pins[i].fd < 0)
        {
            Shutdown();
            return false;
        }
    }
    m_Initialized = true;
    return true;
}

void GpioInputBackend::Shutdown()
{
    for (size_t i = 0; i < m_PinCount; ++i)
    {
        if (m_Pins[i].fd >= 0)
        {
            ::close(m_Pins[i].fd);
            m_Pins[i].fd = -1;
        }
        if (m_Initialized)
        {
            UnexportGpio(m_Pins[i].gpio);
        }
    }
    m_Initialized = false;
}

void GpioInputBackend::Poll(const EmitFn& emit)
{
    if (!m_Initialized)
    {
        return;
    }

    for (size_t i = 0; i < m_PinCount; ++i)
    {
        char value = '0';
        if (::pread(m_Pins[i].fd, &value, 1, 0) != 1)
        {
            continue;
        }

        const bool pressed = (value == '0'); // active low
        if (pressed == m_Pins[i].level)
        {
            continue;
        }
        m_Pins[i].level = pressed;

        SDL_Event event{};
        event.type = pressed ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        event.key.key = m_Pins[i].key;
        event.key.scancode = SDL_SCANCODE_UNKNOWN;
        event.key.repeat = false;
        emit(event);
    }
}
} // namespace flachead::input
