#include "Tween.hpp"

#include <algorithm>

namespace flachead::animation
{
Tween::Tween(float durationSeconds)
    : m_Duration(durationSeconds)
{
}

void Tween::Start(float from, float to, float durationSeconds)
{
    m_From = from;
    m_To = to;
    m_Duration = durationSeconds;
    m_Running = true;
}

float Tween::Value(float elapsedSeconds) const
{
    if (!m_Running || m_Duration <= 0.0f)
    {
        return m_To;
    }

    const float t = std::min(elapsedSeconds / m_Duration, 1.0f);
    return m_From + (m_To - m_From) * t;
}
} // namespace flachead::animation
