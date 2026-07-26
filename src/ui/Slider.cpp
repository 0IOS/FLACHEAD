#include "Slider.hpp"

namespace flachead::ui
{
void Slider::SetValue(float value)
{
    m_Value = value;
}

float Slider::Value() const
{
    return m_Value;
}
} // namespace flachead::ui
