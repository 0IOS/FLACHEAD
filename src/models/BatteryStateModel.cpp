#include "BatteryStateModel.hpp"

namespace flachead::models
{
void BatteryStateModel::SetLevel(int level)
{
    m_Level = level;
}

void BatteryStateModel::SetCharging(bool charging)
{
    m_Charging = charging;
}

int BatteryStateModel::Level() const
{
    return m_Level;
}

bool BatteryStateModel::Charging() const
{
    return m_Charging;
}
} // namespace flachead::models
