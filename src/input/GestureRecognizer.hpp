#pragma once

#include "../math/Vec2.hpp"

#include <functional>
#include <cstdint>

namespace flachead::input
{
enum class GestureType
{
    Tap,
    DoubleTap,
    Hold,
    Swipe,
    DragBegin,
    DragMove,
    DragEnd,
};

struct Gesture
{
    GestureType type{GestureType::Tap};
    Vec2 position;
    Vec2 delta;
    Vec2 startPosition;
    float velocity{0.0f};
    int tapCount{0};
};

struct GestureConfig
{
    uint64_t doubleTapWindowMs{320};
    uint64_t holdDelayMs{420};
    float tapMaxMove{14.0f};
    float swipeMinDistance{60.0f};
    float swipeMinVelocity{350.0f};
    float dragStartDistance{8.0f};
};

// Pure gesture state machine. Feed it press/move/release events with
// millisecond timestamps and it emits Tap / DoubleTap / Hold / Swipe / drag
// gestures. No SDL types involved, so it is fully unit-testable.
class GestureRecognizer
{
public:
    using EmitFn = std::function<void(const Gesture&)>;

    explicit GestureRecognizer(GestureConfig config = {}, EmitFn emit = {});

    void OnPress(const Vec2& position, uint64_t timestampMs);
    void OnMove(const Vec2& position, uint64_t timestampMs);
    void OnRelease(const Vec2& position, uint64_t timestampMs);
    void Cancel();

    bool IsDown() const { return m_Down; }
    int  TapCount() const { return m_TapCount; }

private:
    void Emit(GestureType type, const Vec2& position, const Vec2& delta, float velocity);

    GestureConfig m_Config;
    EmitFn m_Emit;

    bool m_Down{false};
    Vec2 m_DownPosition;
    Vec2 m_LastPosition;
    uint64_t m_DownTimeMs{0};
    uint64_t m_LastReleaseMs{0};
    Vec2 m_LastReleasePosition;
    int m_TapCount{0};
    bool m_HoldEmitted{false};
    bool m_DragStarted{false};
    Vec2 m_DragStart;
};
} // namespace flachead::input
