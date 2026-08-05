#pragma once

#include "../commands/Command.hpp"
#include "../math/Rect.hpp"
#include "../math/Vec2.hpp"
#include "../ui/Canvas.hpp"
#include "ShellServices.hpp"

#include <string_view>

namespace flachead::shell
{
// The operating environment's persistent chrome. Every screen is composed of
// three regions: a top status strip, the primary content region, and the
// bottom navigation bar. Geometry derives from the 240x320 reference portrait
// so the chrome scales up cleanly on larger windows. The nav bar is
// touch-interactive; hardware inputs (d-pad, home button) keep their own
// semantics on top of it.
class ShellChrome
{
public:
    static constexpr float kReferenceWidth = 240.0f;

    explicit ShellChrome(const ShellServices& services);

    // The nav bar maps to these commands, left to right.
    static constexpr flachead::commands::Command kNavCommands[5] = {
        flachead::commands::Command::Back,
        flachead::commands::Command::Launcher,
        flachead::commands::Command::Home,
        flachead::commands::Command::OpenSearch,
        flachead::commands::Command::OpenQueue,
    };

    float Scale(int width) const;
    float StatusHeight(int width) const;
    float NavHeight(int width) const;
    Rect ContentArea(int width, int height) const;

    void DrawStatus(flachead::ui::Canvas& canvas, int width, int height,
                    std::string_view title) const;
    void DrawNav(flachead::ui::Canvas& canvas, int width, int height) const;

    // Hit-testing in logical (pixel) coordinates.
    bool HitNavBar(int width, int height, const Vec2& position) const;
    flachead::commands::Command NavCommandAt(int width, int height, const Vec2& position) const;

private:
    static constexpr std::string_view kNavLabels[5] = {"Back", "Apps", "Home", "Search", "Queue"};

    float NavSlotWidth(int width) const;
    std::string StatusTime() const;

    const ShellServices& m_Services;
};
} // namespace flachead::shell
