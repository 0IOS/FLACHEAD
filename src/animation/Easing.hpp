#pragma once

namespace flachead::animation
{
// Easing curves shared by every animated system. All functions take the
// linear progress t in [0, 1] and return eased progress in [0, 1] (Back
// curves may overshoot slightly past the ends).
enum class Easing
{
    Linear,
    EaseIn,
    EaseOut,
    EaseInOut,
    EaseInQuad,
    EaseOutQuad,
    EaseInOutQuad,
    EaseInCubic,
    EaseOutCubic,
    EaseInOutCubic,
    EaseOutBack,
    EaseOutElastic,
};

float ApplyEasing(Easing easing, float t);
} // namespace flachead::animation
