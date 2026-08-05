#include "Easing.hpp"

#include <algorithm>
#include <cmath>

namespace flachead::animation
{
namespace
{
float Clamp01(float t)
{
    return std::clamp(t, 0.0f, 1.0f);
}

float BackOut(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

float ElasticOut(float t)
{
    if (t == 0.0f || t == 1.0f)
    {
        return t;
    }
    return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * (2.0f * 3.14159265f) / 3.0f) + 1.0f;
}
} // namespace

float ApplyEasing(Easing easing, float t)
{
    switch (easing)
    {
        case Easing::Linear:
            return t;
        case Easing::EaseIn:
            return t * t;
        case Easing::EaseOut:
            return 1.0f - (1.0f - t) * (1.0f - t);
        case Easing::EaseInOut:
            return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
        case Easing::EaseInQuad:
            return t * t;
        case Easing::EaseOutQuad:
            return 1.0f - (1.0f - t) * (1.0f - t);
        case Easing::EaseInOutQuad:
            return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
        case Easing::EaseInCubic:
            return t * t * t;
        case Easing::EaseOutCubic:
            return 1.0f - std::pow(1.0f - t, 3.0f);
        case Easing::EaseInOutCubic:
            return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
        case Easing::EaseOutBack:
            return BackOut(Clamp01(t));
        case Easing::EaseOutElastic:
            return ElasticOut(Clamp01(t));
    }
    return t;
}
} // namespace flachead::animation
