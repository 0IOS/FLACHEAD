#include "../src/animation/AnimationManager.hpp"
#include "../src/commands/CommandCenter.hpp"
#include "../src/focus/FocusManager.hpp"
#include "../src/input/GestureRecognizer.hpp"
#include "../src/input/InputEvent.hpp"
#include "../src/layout/BoxLayout.hpp"
#include "../src/layout/GridLayout.hpp"
#include "../src/layout/LayoutEngine.hpp"
#include "../src/math/Rect.hpp"
#include "../src/math/Vec2.hpp"
#include "test_util.hpp"

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
} // namespace

int main()
{
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
    return Finish();
}
