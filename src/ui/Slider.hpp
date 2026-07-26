#pragma once

#include "Widget.hpp"

namespace flachead::ui
{
class Slider : public Widget
{
public:
    void SetValue(float value);
    float Value() const;

private:
    float m_Value{0.0f};
};
} // namespace flachead::ui
