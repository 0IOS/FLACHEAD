#include "Animator.hpp"

namespace flachead::animation
{
Animator::Animator(TickHandler tickHandler)
    : m_TickHandler(std::move(tickHandler))
{
}

void Animator::Tick(float deltaSeconds)
{
    if (m_TickHandler)
    {
        m_TickHandler(deltaSeconds);
    }
}
} // namespace flachead::animation
