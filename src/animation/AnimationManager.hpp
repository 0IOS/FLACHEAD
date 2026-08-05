#pragma once

#include "../math/Color.hpp"
#include "../math/Vec2.hpp"
#include "Easing.hpp"

#include <cstdint>
#include <functional>
#include <vector>

namespace flachead::animation
{
// Central animation engine. Screens and widgets request tweens and get back
// an id; the manager ticks them all once per frame and calls back with the
// interpolated value. Supports float, Vec2 and Color properties with a shared
// easing set. Animations are removed automatically when they finish.
class AnimationManager
{
public:
    using FloatCallback  = std::function<void(float)>;
    using Vec2Callback   = std::function<void(const Vec2&)>;
    using ColorCallback  = std::function<void(const Color&)>;
    using CompleteCallback = std::function<void()>;

    AnimationManager() = default;
    ~AnimationManager() = default;

    AnimationManager(const AnimationManager&) = delete;
    AnimationManager& operator=(const AnimationManager&) = delete;

    uint64_t AnimateFloat(float from, float to, float durationSeconds,
                          Easing easing, FloatCallback onUpdate,
                          CompleteCallback onComplete = {});
    uint64_t AnimateVec2(const Vec2& from, const Vec2& to, float durationSeconds,
                         Easing easing, Vec2Callback onUpdate,
                         CompleteCallback onComplete = {});
    uint64_t AnimateColor(const Color& from, const Color& to, float durationSeconds,
                          Easing easing, ColorCallback onUpdate,
                          CompleteCallback onComplete = {});

    void Cancel(uint64_t id);
    void CancelAll();

    bool IsActive(uint64_t id) const;
    int  ActiveCount() const { return static_cast<int>(m_Animations.size()); }

    void Tick(float deltaSeconds);

private:
    struct Animation
    {
        uint64_t id{0};
        enum class Kind
        {
            Float,
            Vec2,
            Color,
        };
        Kind kind{Kind::Float};
        float elapsed{0.0f};
        float duration{0.0f};
        Easing easing{Easing::Linear};

        float floatFrom{0.0f};
        float floatTo{0.0f};
        Vec2 vec2From;
        Vec2 vec2To;
        Color colorFrom{0, 0, 0};
        Color colorTo{0, 0, 0};

        FloatCallback  onUpdate;
        Vec2Callback   vec2OnUpdate;
        ColorCallback  colorOnUpdate;
        CompleteCallback onComplete;
    };

    void Advance(Animation& animation);

    std::vector<Animation> m_Animations;
    uint64_t m_NextId{1};
};
} // namespace flachead::animation
