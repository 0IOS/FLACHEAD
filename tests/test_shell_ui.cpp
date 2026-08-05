#include "../src/ui/palette/Palette.hpp"

#include "../src/animation/AnimationManager.hpp"
#include "../src/commands/CommandCenter.hpp"
#include "../src/focus/FocusManager.hpp"
#include "../src/input/GestureRecognizer.hpp"
#include "../src/input/InputEvent.hpp"
#include "../src/input/InputManager.hpp"
#include "../src/layout/BoxLayout.hpp"
#include "../src/layout/GridLayout.hpp"
#include "../src/layout/LayoutEngine.hpp"
#include "../src/math/Rect.hpp"
#include "../src/math/Vec2.hpp"
#include "../src/shell/ScreenTransitionManager.hpp"
#include "test_util.hpp"

#include <SDL3/SDL.h>

#include <cmath>

using flachead::animation::AnimationManager;
using flachead::animation::Easing;
using flachead::commands::Command;
using flachead::commands::CommandCenter;
using flachead::focus::FocusManager;
using flachead::input::Gesture;
using flachead::input::GestureConfig;
using flachead::input::GestureRecognizer;
using flachead::input::GestureType;
using flachead::layout::BoxLayout;
using flachead::layout::CrossAlign;
using flachead::layout::GridLayout;
using flachead::layout::Orientation;

namespace
{
float Approx(float a, float b)
{
    return std::fabs(a - b) < 0.01f;
}

void TestBoxLayoutStretch()
{
    BoxLayout layout(Orientation::Horizontal);
    layout.Spacing(4.0f).PaddingAll(6.0f);
    std::vector<Vec2> preferred = {Vec2{0.0f, 20.0f}, Vec2{40.0f, 20.0f}};
    const auto result = layout.Layout(preferred, Vec2{100.0f, 32.0f});

    Check(result.rects.size() == 2, "box has two rects");
    Check(Approx(result.rects[0].position.x, 6.0f), "stretch child starts after padding");
    Check(Approx(result.rects[0].size.x, 44.0f), "stretch child fills remaining main axis (88 - 40 - 4)");
    Check(Approx(result.rects[0].size.y, 20.0f), "stretch child takes cross axis size");
    Check(Approx(result.rects[1].position.x, 6.0f + 44.0f + 4.0f), "fixed child follows stretch+spacing");
    Check(Approx(result.rects[1].size.x, 40.0f), "fixed child keeps preferred size");
}

void TestBoxLayoutAlignment()
{
    BoxLayout layout(Orientation::Vertical);
    layout.MainAxis(flachead::layout::AxisAlign::Center);
    layout.CrossAxis(CrossAlign::Center);
    std::vector<Vec2> preferred = {Vec2{20.0f, 30.0f}};
    const auto result = layout.Layout(preferred, Vec2{100.0f, 100.0f});

    Check(Approx(result.rects[0].position.x, (100.0f - 20.0f) * 0.5f), "cross axis centered x");
    Check(Approx(result.rects[0].position.y, (100.0f - 30.0f) * 0.5f), "main axis centered y");
}

void TestBoxLayoutMeasure()
{
    BoxLayout layout(Orientation::Horizontal);
    layout.Spacing(5.0f).PaddingAll(10.0f);
    std::vector<Vec2> preferred = {Vec2{20.0f, 5.0f}, Vec2{30.0f, 8.0f}};
    const auto measured = layout.Measure(preferred);

    Check(Approx(measured.x, 20.0f + 30.0f + 5.0f + 20.0f), "measure sums main axis + spacing + padding");
    Check(Approx(measured.y, 8.0f + 20.0f), "measure takes max cross + vertical padding");
}

void TestGridLayout()
{
    GridLayout grid;
    grid.Columns(2).Spacing(10.0f);
    std::vector<Vec2> preferred = {Vec2{50.0f, 40.0f}, Vec2{50.0f, 40.0f}, Vec2{50.0f, 40.0f}};
    const auto result = grid.Layout(preferred, Vec2{110.0f, 200.0f});

    Check(result.rects.size() == 3, "grid has three rects");
    Check(Approx(result.rects[0].size.x, 50.0f), "grid cell width splits available space");
    Check(Approx(result.rects[1].position.x, 60.0f), "second column starts after spacing");
    Check(Approx(result.rects[2].position.y, 55.0f + 50.0f), "third item wraps to second row");
}

void TestAnimationManagerFloat()
{
    AnimationManager manager;
    float value = 0.0f;
    bool completed = false;
    const auto id = manager.AnimateFloat(0.0f, 10.0f, 1.0f, Easing::Linear,
                                         [&value](float v) { value = v; },
                                         [&completed]() { completed = true; });
    Check(manager.IsActive(id), "animation active after start");

    manager.Tick(0.5f);
    Check(Approx(value, 5.0f), "linear tween reaches midpoint");
    manager.Tick(0.5f);
    Check(Approx(value, 10.0f), "linear tween reaches end");
    Check(completed, "completion callback fired");
    Check(!manager.IsActive(id), "finished animation removed");
}

void TestAnimationManagerEasing()
{
    AnimationManager manager;
    float value = 0.0f;
    manager.AnimateFloat(0.0f, 1.0f, 1.0f, Easing::EaseInQuad,
                         [&value](float v) { value = v; });
    manager.Tick(0.5f);
    Check(value < 0.5f, "ease-in stays below linear midpoint");
    manager.CancelAll();
    Check(manager.ActiveCount() == 0, "cancel all empties manager");
}

void TestAnimationManagerColor()
{
    AnimationManager manager;
    Color color{0, 0, 0, 255};
    manager.AnimateColor(Color{0, 0, 0, 255},
                         Color{255, 255, 255, 255},
                         1.0f, Easing::Linear,
                         [&color](const Color& c) { color = c; });
    manager.Tick(1.0f);
    Check(color.r == 255 && color.g == 255 && color.b == 255, "color reaches target");
}

void TestGestureTap()
{
    GestureType last = GestureType::Tap;
    GestureRecognizer rec(GestureConfig{}, [&last](const Gesture& g) { last = g.type; });

    rec.OnPress(Vec2{10.0f, 10.0f}, 0);
    rec.OnRelease(Vec2{10.0f, 10.0f}, 40);
    Check(last == GestureType::Tap, "press+release within window is a tap");
}

void TestGestureDoubleTap()
{
    GestureType last = GestureType::Tap;
    GestureRecognizer rec(GestureConfig{}, [&last](const Gesture& g) { last = g.type; });

    rec.OnPress(Vec2{10.0f, 10.0f}, 0);
    rec.OnRelease(Vec2{10.0f, 10.0f}, 40);
    rec.OnPress(Vec2{11.0f, 11.0f}, 80);
    rec.OnRelease(Vec2{11.0f, 11.0f}, 120);
    Check(last == GestureType::DoubleTap, "two taps within window is a double tap");
}

void TestGestureHold()
{
    GestureType last = GestureType::Tap;
    GestureRecognizer rec(GestureConfig{}, [&last](const Gesture& g) { last = g.type; });

    rec.OnPress(Vec2{10.0f, 10.0f}, 0);
    rec.OnMove(Vec2{10.0f, 10.0f}, 500);
    Check(last == GestureType::Hold, "stationary press beyond delay is a hold");
}

void TestGestureSwipe()
{
    GestureType last = GestureType::Tap;
    GestureRecognizer rec(GestureConfig{}, [&last](const Gesture& g) { last = g.type; });

    rec.OnPress(Vec2{0.0f, 10.0f}, 0);
    rec.OnMove(Vec2{100.0f, 10.0f}, 60);
    rec.OnRelease(Vec2{100.0f, 10.0f}, 100);
    Check(last == GestureType::Swipe, "fast horizontal motion is a swipe");
}

void TestGestureDrag()
{
    GestureType last = GestureType::Tap;
    GestureRecognizer rec(GestureConfig{}, [&last](const Gesture& g) { last = g.type; });

    rec.OnPress(Vec2{10.0f, 10.0f}, 0);
    rec.OnMove(Vec2{30.0f, 20.0f}, 1000);
    Check(last == GestureType::DragMove, "slow move beyond start distance is a drag");
}

void TestFocusManager()
{
    FocusManager focus;
    focus.Register("a", Rect{10.0f, 10.0f, 100.0f, 40.0f});
    focus.Register("b", Rect{10.0f, 60.0f, 100.0f, 40.0f});
    focus.Register("c", Rect{130.0f, 10.0f, 100.0f, 40.0f});

    focus.SetFocus("a");
    focus.Move(Command::Down);
    Check(focus.FocusedId() == "b", "down from a moves to b");
    focus.Move(Command::Right);
    Check(focus.FocusedId() == "c", "right from b moves to c");
    focus.Move(Command::Left);
    Check(focus.FocusedId() == "a", "left from c returns to a");
}

void TestCommandCenter()
{
    CommandCenter center;
    int first = 0;
    int second = 0;

    center.Register([&first](Command command) {
        if (command == Command::PlayPause)
        {
            ++first;
            return true;
        }
        return false;
    }, 100);
    center.Register([&second](Command command) {
        if (command == Command::PlayPause)
        {
            ++second;
            return true;
        }
        return false;
    }, 50);

    Check(center.Dispatch(Command::PlayPause), "dispatch claimed by high priority handler");
    Check(first == 1 && second == 0, "only highest priority handler ran");
    Check(!center.Dispatch(Command::Back), "unclaimed command returns false");
}

void TestKeyToCommand()
{
    Check(flachead::input::KeyToCommand(SDLK_UP) == Command::Up, "arrow up maps to Up");
    Check(flachead::input::KeyToCommand(SDLK_ESCAPE) == Command::Back, "escape maps to Back");
    Check(flachead::input::KeyToCommand(SDLK_MEDIA_NEXT_TRACK) == Command::Next,
          "media next maps to Next");
}

void TestParseHex()
{
    const Color c = flachead::palette::ParseHex("#7c3aed");
    Check(c.r == 0x7c && c.g == 0x3a && c.b == 0xed && c.a == 255, "parses #rrggbb");
    const Color rgba = flachead::palette::ParseHex("#7c3aed80");
    Check(rgba.a == 0x80, "parses #rrggbbaa");
    const Color bad = flachead::palette::ParseHex("nonsense");
    Check(bad.r == 0 && bad.g == 0 && bad.b == 0, "malformed hex returns black");
}

void TestHslRoundTrip()
{
    const Color original{100, 150, 200, 255};
    const flachead::palette::Hsl hsl = flachead::palette::ColorUtil::ToHsl(original);
    const Color round = flachead::palette::ColorUtil::FromHsl(hsl);
    Check(std::abs(static_cast<int>(round.r) - original.r) <= 1 &&
          std::abs(static_cast<int>(round.g) - original.g) <= 1 &&
          std::abs(static_cast<int>(round.b) - original.b) <= 1,
          "hsl round trip within 1 channel unit");
}

void TestColorMix()
{
    const Color mixed = flachead::palette::ColorUtil::Mix(Color::Black, Color::White, 0.5f);
    Check(mixed.r == 128 && mixed.g == 128 && mixed.b == 128, "mix midpoint is gray");
    const Color lightened = flachead::palette::ColorUtil::Lighten(Color::Black, 0.5f);
    Check(lightened.r > 100, "lightening black raises channels");
}

void TestContrastRatio()
{
    const float ratio = flachead::palette::ColorUtil::ContrastRatio(Color::Black, Color::White);
    Check(ratio > 15.0f, "black/white contrast is high");
    const float same = flachead::palette::ColorUtil::ContrastRatio(Color::White, Color::White);
    Check(same > 0.9f && same < 1.1f, "identical colors have ratio ~1");
}

void TestPaletteDerivation()
{
    const flachead::palette::Palette dark = flachead::palette::Palette::FromSeed(Color{124, 58, 237}, true);
    Check(dark.accent.r == 124 && dark.accent.g == 58 && dark.accent.b == 237, "accent preserved");
    const float bgLum = flachead::palette::ColorUtil::Luminance(dark.background);
    const float fgLum = flachead::palette::ColorUtil::Luminance(dark.foreground);
    Check(bgLum < 0.05f, "dark theme has dark background");
    Check(fgLum > 0.7f, "dark theme has light foreground");
    Check(flachead::palette::ColorUtil::ContrastRatio(dark.foreground, dark.background) > 10.0f,
          "foreground contrasts with background");

    const flachead::palette::Palette light = flachead::palette::Palette::FromSeed(Color{124, 58, 237}, false);
    Check(flachead::palette::ColorUtil::Luminance(light.background) > 0.8f, "light theme has light background");
    Check(flachead::palette::ColorUtil::Luminance(light.foreground) < 0.05f, "light theme has dark foreground");
}

void TestDominantColors()
{
    std::vector<uint8_t> pixels;
    constexpr int width = 64;
    constexpr int height = 64;
    pixels.resize(static_cast<std::size_t>(width) * height * 4);
    for (std::size_t i = 0; i < pixels.size(); i += 4)
    {
        pixels[i] = 220;
        pixels[i + 1] = 40;
        pixels[i + 2] = 80;
        pixels[i + 3] = 255;
    }
    const auto colors = flachead::palette::ExtractDominantColors(pixels.data(), width, height);
    Check(!colors.empty(), "dominant colors found");
    if (!colors.empty())
    {
        const auto first = colors.front();
        const float hue = flachead::palette::ColorUtil::ToHsl(first).h;
        Check(hue < 20.0f || hue > 340.0f, "dominant hue near red for red image");
    }
}

void TestScreenTransitionFade()
{
    flachead::shell::ScreenTransitionManager transition;
    Check(!transition.Active(), "transition idle by default");
    Check(!transition.NeedsRender(), "no render demand when idle");

    transition.BeginFade(0.2f);
    Check(transition.Active(), "transition starts active");
    Check(Approx(transition.Alpha(), 1.0f), "fade starts fully covered");

    transition.Update(0.1f);
    const float halfway = transition.Alpha();
    Check(halfway > 0.0f && halfway < 1.0f, "fade eases through mid values");

    transition.Update(0.2f);
    Check(!transition.Active(), "transition finishes");
    Check(Approx(transition.Alpha(), 0.0f), "fade reaches transparent");
    Check(!transition.NeedsRender(), "no render demand after finish");
}

void TestHomeTapOpensLauncher()
{
    flachead::input::GestureConfig config;
    config.doubleTapWindowMs = 0;
    config.holdDelayMs = 1000;
    flachead::input::InputManager manager(config);
    manager.Initialize();

    std::vector<flachead::input::InputEvent> inputs;
    std::vector<flachead::commands::Command> commands;
    manager.SetInputEventCallback([&](const flachead::input::InputEvent& event) { inputs.push_back(event); });
    manager.SetCommandCallback([&](flachead::commands::Command command) { commands.push_back(command); });

    SDL_Event down{};
    down.type = SDL_EVENT_KEY_DOWN;
    down.key.key = SDLK_HOME;
    SDL_Event up{};
    up.type = SDL_EVENT_KEY_UP;
    up.key.key = SDLK_HOME;
    manager.HandleEvent(down);
    manager.HandleEvent(up);

    Check(inputs.size() == 1, "single home tap emits one signal event");
    if (!inputs.empty())
    {
        Check(inputs[0].command == flachead::commands::Command::Launcher,
              "pending single tap signals the launcher command");
    }

    manager.Update();
    Check(std::find(commands.begin(), commands.end(), flachead::commands::Command::Launcher) != commands.end(),
          "launcher command fires after the single-tap window");
}

void TestHomeDoubleTapGoesHome()
{
    flachead::input::GestureConfig config;
    config.doubleTapWindowMs = 1000;
    config.holdDelayMs = 1000;
    flachead::input::InputManager manager(config);
    manager.Initialize();

    std::vector<flachead::commands::Command> commands;
    manager.SetCommandCallback([&](flachead::commands::Command command) { commands.push_back(command); });

    SDL_Event down{};
    down.type = SDL_EVENT_KEY_DOWN;
    down.key.key = SDLK_HOME;
    SDL_Event up{};
    up.type = SDL_EVENT_KEY_UP;
    up.key.key = SDLK_HOME;
    manager.HandleEvent(down);
    manager.HandleEvent(up);
    manager.HandleEvent(down);
    manager.HandleEvent(up);

    Check(std::find(commands.begin(), commands.end(), flachead::commands::Command::Home) != commands.end(),
          "double home tap emits Home");
}

void TestHomeHoldOpensTaskOverview()
{
    flachead::input::GestureConfig config;
    config.doubleTapWindowMs = 1000;
    config.holdDelayMs = 0;
    flachead::input::InputManager manager(config);
    manager.Initialize();

    std::vector<flachead::commands::Command> commands;
    manager.SetCommandCallback([&](flachead::commands::Command command) { commands.push_back(command); });

    SDL_Event down{};
    down.type = SDL_EVENT_KEY_DOWN;
    down.key.key = SDLK_HOME;
    SDL_Event up{};
    up.type = SDL_EVENT_KEY_UP;
    up.key.key = SDLK_HOME;
    manager.HandleEvent(down);
    manager.HandleEvent(up);

    Check(std::find(commands.begin(), commands.end(), flachead::commands::Command::TaskOverview)
              != commands.end(),
          "home hold emits TaskOverview");
}
} // namespace

int main()
{
    SDL_Init(0);
    RunTest("BoxLayout stretch/fixed/spacing", TestBoxLayoutStretch);
    RunTest("BoxLayout alignment", TestBoxLayoutAlignment);
    RunTest("BoxLayout measure", TestBoxLayoutMeasure);
    RunTest("GridLayout rows/columns", TestGridLayout);
    RunTest("AnimationManager float tween", TestAnimationManagerFloat);
    RunTest("AnimationManager easing", TestAnimationManagerEasing);
    RunTest("AnimationManager color tween", TestAnimationManagerColor);
    RunTest("Gesture tap", TestGestureTap);
    RunTest("Gesture double-tap", TestGestureDoubleTap);
    RunTest("Gesture hold", TestGestureHold);
    RunTest("Gesture swipe", TestGestureSwipe);
    RunTest("Gesture drag", TestGestureDrag);
    RunTest("FocusManager navigation", TestFocusManager);
    RunTest("CommandCenter priority", TestCommandCenter);
    RunTest("KeyToCommand mapping", TestKeyToCommand);
    RunTest("Palette parse hex", TestParseHex);
    RunTest("Palette hsl round trip", TestHslRoundTrip);
    RunTest("Palette color mix", TestColorMix);
    RunTest("Palette contrast ratio", TestContrastRatio);
    RunTest("Palette derivation", TestPaletteDerivation);
    RunTest("Palette dominant colors", TestDominantColors);
    RunTest("Screen transition fade", TestScreenTransitionFade);
    RunTest("Home tap opens launcher", TestHomeTapOpensLauncher);
    RunTest("Home double tap goes home", TestHomeDoubleTapGoesHome);
    RunTest("Home hold opens task overview", TestHomeHoldOpensTaskOverview);
    return Finish();
}
