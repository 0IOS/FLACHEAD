#include "ProgressBar.hpp"

namespace flachead::ui
{
void ProgressBar::SetValue(float value)
{
    m_Value = value;
}

float ProgressBar::Value() const
{
    return m_Value;
}
} // namespace flachead::ui
