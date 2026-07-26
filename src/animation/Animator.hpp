#pragma once

#include <functional>

namespace flachead::animation
{
class Animator
{
public:
    using TickHandler = std::function<void(float)>;

    explicit Animator(TickHandler tickHandler);
    void Tick(float deltaSeconds);

private:
    TickHandler m_TickHandler;
};
} // namespace flachead::animation
