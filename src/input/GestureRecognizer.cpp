#include "GestureRecognizer.hpp"

#include <algorithm>
#include <cmath>

namespace flachead::input
{
namespace
{
float Distance(const Vec2& a, const Vec2& b)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}
} // namespace

GestureRecognizer::GestureRecognizer(GestureConfig config, EmitFn emit)
    : m_Config(config),
      m_Emit(std::move(emit))
{
}

void GestureRecognizer::Emit(GestureType type, const Vec2& position, const Vec2& delta, float velocity)
{
    if (!m_Emit)
    {
        return;
    }
    Gesture gesture;
    gesture.type = type;
    gesture.position = position;
    gesture.delta = delta;
    gesture.startPosition = m_DownPosition;
    gesture.velocity = velocity;
    gesture.tapCount = m_TapCount;
    m_Emit(gesture);
}

void GestureRecognizer::OnPress(const Vec2& position, uint64_t timestampMs)
{
    m_Down = true;
    m_DownPosition = position;
    m_LastPosition = position;
    m_DownTimeMs = timestampMs;
    m_HoldEmitted = false;
    m_DragStarted = false;
    m_DragStart = position;
}

void GestureRecognizer::OnMove(const Vec2& position, uint64_t timestampMs)
{
    if (!m_Down)
    {
        return;
    }
    const float moved = Distance(position, m_DownPosition);

    if (!m_HoldEmitted && timestampMs - m_DownTimeMs >= m_Config.holdDelayMs && moved < m_Config.tapMaxMove)
    {
        m_HoldEmitted = true;
        m_TapCount = 0;
        Emit(GestureType::Hold, position, Vec2{}, 0.0f);
    }

    if (!m_DragStarted && moved >= m_Config.dragStartDistance)
    {
        m_DragStarted = true;
        m_TapCount = 0;
        Emit(GestureType::DragBegin, position,
             Vec2{position.x - m_DragStart.x, position.y - m_DragStart.y}, 0.0f);
    }

    if (m_DragStarted)
    {
        Emit(GestureType::DragMove, position,
             Vec2{position.x - m_LastPosition.x, position.y - m_LastPosition.y}, 0.0f);
    }
    m_LastPosition = position;
}

void GestureRecognizer::OnRelease(const Vec2& position, uint64_t timestampMs)
{
    if (!m_Down)
    {
        return;
    }
    m_Down = false;
    const float moved = Distance(position, m_DownPosition);
    const float pressMs = static_cast<float>(timestampMs - m_DownTimeMs);

    if (m_DragStarted)
    {
        const Vec2 total{position.x - m_DragStart.x, position.y - m_DragStart.y};
        const float totalDist = std::sqrt(total.x * total.x + total.y * total.y);
        const float pressSeconds = std::max(0.001f, pressMs / 1000.0f);
        const float velocity = totalDist / pressSeconds;
        if (totalDist >= m_Config.swipeMinDistance && velocity >= m_Config.swipeMinVelocity)
        {
            Emit(GestureType::Swipe, position, total, velocity);
        }
        else
        {
            Emit(GestureType::DragEnd, position, total, velocity);
        }
        m_DragStarted = false;
        return;
    }

    const bool withinTap = moved <= m_Config.tapMaxMove;
    const bool withinWindow = timestampMs - m_LastReleaseMs <= m_Config.doubleTapWindowMs &&
                              Distance(position, m_LastReleasePosition) <= m_Config.tapMaxMove * 2.0f;

    if (withinTap)
    {
        m_TapCount = withinWindow ? m_TapCount + 1 : 1;
        if (m_TapCount >= 2)
        {
            Emit(GestureType::DoubleTap, position, Vec2{}, 0.0f);
            m_TapCount = 0;
        }
        else
        {
            Emit(GestureType::Tap, position, Vec2{}, 0.0f);
        }
    }
    else
    {
        m_TapCount = 0;
    }

    m_LastReleaseMs = timestampMs;
    m_LastReleasePosition = position;
}

void GestureRecognizer::Cancel()
{
    m_Down = false;
    m_DragStarted = false;
    m_TapCount = 0;
}
} // namespace flachead::input
