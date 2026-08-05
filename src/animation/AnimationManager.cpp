#include "AnimationManager.hpp"

#include <algorithm>
#include <cmath>

namespace flachead::animation
{
namespace
{
float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

Color LerpColor(const Color& a, const Color& b, float t)
{
    auto lerp8 = [t](uint8_t x, uint8_t y) {
        return static_cast<uint8_t>(std::round(Lerp(static_cast<float>(x),
                                                    static_cast<float>(y), t)));
    };
    return Color{lerp8(a.r, b.r), lerp8(a.g, b.g), lerp8(a.b, b.b), lerp8(a.a, b.a)};
}
} // namespace

uint64_t AnimationManager::AnimateFloat(float from, float to, float durationSeconds,
                                        Easing easing, FloatCallback onUpdate,
                                        CompleteCallback onComplete)
{
    if (durationSeconds <= 0.0f)
    {
        if (onUpdate)
        {
            onUpdate(to);
        }
        if (onComplete)
        {
            onComplete();
        }
        return 0;
    }
    Animation animation;
    animation.id = m_NextId++;
    animation.kind = Animation::Kind::Float;
    animation.duration = durationSeconds;
    animation.easing = easing;
    animation.floatFrom = from;
    animation.floatTo = to;
    animation.onUpdate = std::move(onUpdate);
    animation.onComplete = std::move(onComplete);
    m_Animations.push_back(std::move(animation));
    return m_Animations.back().id;
}

uint64_t AnimationManager::AnimateVec2(const Vec2& from, const Vec2& to, float durationSeconds,
                                       Easing easing, Vec2Callback onUpdate,
                                       CompleteCallback onComplete)
{
    if (durationSeconds <= 0.0f)
    {
        if (onUpdate)
        {
            onUpdate(to);
        }
        if (onComplete)
        {
            onComplete();
        }
        return 0;
    }
    Animation animation;
    animation.id = m_NextId++;
    animation.kind = Animation::Kind::Vec2;
    animation.duration = durationSeconds;
    animation.easing = easing;
    animation.vec2From = from;
    animation.vec2To = to;
    animation.vec2OnUpdate = std::move(onUpdate);
    animation.onComplete = std::move(onComplete);
    m_Animations.push_back(std::move(animation));
    return m_Animations.back().id;
}

uint64_t AnimationManager::AnimateColor(const Color& from, const Color& to, float durationSeconds,
                                        Easing easing, ColorCallback onUpdate,
                                        CompleteCallback onComplete)
{
    if (durationSeconds <= 0.0f)
    {
        if (onUpdate)
        {
            onUpdate(to);
        }
        if (onComplete)
        {
            onComplete();
        }
        return 0;
    }
    Animation animation;
    animation.id = m_NextId++;
    animation.kind = Animation::Kind::Color;
    animation.duration = durationSeconds;
    animation.easing = easing;
    animation.colorFrom = from;
    animation.colorTo = to;
    animation.colorOnUpdate = std::move(onUpdate);
    animation.onComplete = std::move(onComplete);
    m_Animations.push_back(std::move(animation));
    return m_Animations.back().id;
}

void AnimationManager::Cancel(uint64_t id)
{
    if (id == 0)
    {
        return;
    }
    m_Animations.erase(std::remove_if(m_Animations.begin(), m_Animations.end(),
                                      [id](const Animation& a) { return a.id == id; }),
                       m_Animations.end());
}

void AnimationManager::CancelAll()
{
    m_Animations.clear();
}

bool AnimationManager::IsActive(uint64_t id) const
{
    if (id == 0)
    {
        return false;
    }
    for (const auto& animation : m_Animations)
    {
        if (animation.id == id)
        {
            return true;
        }
    }
    return false;
}

void AnimationManager::Advance(Animation& animation)
{
    const float t = std::clamp(animation.elapsed / animation.duration, 0.0f, 1.0f);
    const float eased = ApplyEasing(animation.easing, t);
    switch (animation.kind)
    {
        case Animation::Kind::Float:
            if (animation.onUpdate)
            {
                animation.onUpdate(Lerp(animation.floatFrom, animation.floatTo, eased));
            }
            break;
        case Animation::Kind::Vec2:
            if (animation.vec2OnUpdate)
            {
                animation.vec2OnUpdate(Vec2{Lerp(animation.vec2From.x, animation.vec2To.x, eased),
                                            Lerp(animation.vec2From.y, animation.vec2To.y, eased)});
            }
            break;
        case Animation::Kind::Color:
            if (animation.colorOnUpdate)
            {
                animation.colorOnUpdate(LerpColor(animation.colorFrom, animation.colorTo, eased));
            }
            break;
    }
}

void AnimationManager::Tick(float deltaSeconds)
{
    if (m_Animations.empty())
    {
        return;
    }

    for (auto& animation : m_Animations)
    {
        if (animation.duration <= 0.0f)
        {
            continue;
        }
        animation.elapsed += deltaSeconds;
        const bool finished = animation.elapsed >= animation.duration;
        if (finished)
        {
            animation.elapsed = animation.duration;
        }
        Advance(animation);
        if (finished && animation.onComplete)
        {
            animation.onComplete();
        }
    }

    m_Animations.erase(std::remove_if(m_Animations.begin(), m_Animations.end(),
                                      [](const Animation& a) {
                                          return a.duration > 0.0f &&
                                                 a.elapsed >= a.duration;
                                      }),
                       m_Animations.end());
}
} // namespace flachead::animation
