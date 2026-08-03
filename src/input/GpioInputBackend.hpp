#pragma once

#include "InputBackend.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace flachead::input
{
// GPIO button backend for the Pi Zero W. Reads /sys/class/gpio (sysfs, no
// extra dependencies) and synthesizes SDL keyboard events, so the rest of the
// app behaves exactly as if a keyboard was attached. Buttons are active-low
// (1.8 V/3.3 V GPIO pulled up, pressed = 0), which is the common Pi wiring.
//
// Wiring (BCM numbers), matching the launcher's navigate/accept/back scheme:
//   17  up      22  left     24  accept (enter)
//   27  down    23  right    25  back   (escape)
//   5  play/pause, 6  next track, 13 previous track
//
// On hosts without a GPIO controller (desktop, QEMU) Initialize() fails and
// the caller falls back to the SDL backend.
class GpioInputBackend final : public InputBackend
{
public:
    struct Pin
    {
        int gpio;
        SDL_Keycode key;
    };

    explicit GpioInputBackend(std::vector<Pin> pins = DefaultPins());

    bool Initialize() override;
    void Shutdown() override;
    void Poll(const EmitFn& emit) override;
    const char* Name() const override { return "gpio"; }

    static std::vector<Pin> DefaultPins();

private:
    struct PinState
    {
        int gpio;
        SDL_Keycode key;
        int fd{-1};
        bool level{false};
    };

    static bool ExportGpio(int gpio);
    static bool UnexportGpio(int gpio);
    static int OpenValueFile(int gpio);

    std::array<PinState, 12> m_Pins{};
    size_t m_PinCount{0};
    bool m_Initialized{false};
};
} // namespace flachead::input
