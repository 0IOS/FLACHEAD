#include "AnimationPlayer.hpp"

#include <utility>

namespace flachead::animation
{
void AnimationPlayer::Play(Tween tween)
{
    m_Tween = std::move(tween);
    m_Playing = true;
}

float AnimationPlayer::Value(float elapsedSeconds) const
{
    if (!m_Playing)
    {
        return 1.0f;
    }

    return m_Tween.Value(elapsedSeconds);
}
} // namespace flachead::animation
