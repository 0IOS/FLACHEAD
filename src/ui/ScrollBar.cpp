#include "ScrollBar.hpp"

namespace flachead::ui
{
void ScrollBar::SetValue(float value)
{
    m_Value = value;
}

float ScrollBar::Value() const
{
    return m_Value;
}
} // namespace flachead::ui
