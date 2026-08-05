#include "ScreenTransitionManager.hpp"

#include <algorithm>
#include <cmath>

namespace flachead::shell
{
namespace
{
constexpr float kEaseOutPower = 2.0f;
} // namespace

void ScreenTransitionManager::BeginFade(float durationSeconds)
{
    m_Duration = std::max(0.05f, durationSeconds);
    m_Elapsed = 0.0f;
    m_Alpha = 1.0f;
}

void ScreenTransitionManager::Update(float deltaSeconds)
{
    if (m_Elapsed >= m_Duration)
    {
        return;
    }

    m_Elapsed += std::max(0.0f, deltaSeconds);
    const float progress = std::clamp(m_Elapsed / m_Duration, 0.0f, 1.0f);
    const float eased = 1.0f - std::pow(1.0f - progress, kEaseOutPower);
    m_Alpha = 1.0f - eased;
}
} // namespace flachead::shell
