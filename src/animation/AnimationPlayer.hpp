#pragma once

#include "Tween.hpp"

namespace flachead::animation
{
class AnimationPlayer
{
public:
    void Play(Tween tween);
    float Value(float elapsedSeconds) const;

private:
    Tween m_Tween;
    bool m_Playing{false};
};
} // namespace flachead::animation
