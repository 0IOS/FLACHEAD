#pragma once

namespace flachead::animation
{
class Tween
{
public:
    Tween() = default;
    explicit Tween(float durationSeconds);

    void Start(float from, float to, float durationSeconds);
    float Value(float elapsedSeconds) const;

private:
    float m_From{0.0f};
    float m_To{1.0f};
    float m_Duration{0.0f};
    bool m_Running{false};
};
} // namespace flachead::animation
