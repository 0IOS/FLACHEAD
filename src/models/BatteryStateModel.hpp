#pragma once

namespace flachead::models
{
class BatteryStateModel
{
public:
    BatteryStateModel() = default;

    void SetLevel(int level);
    void SetCharging(bool charging);

    int Level() const;
    bool Charging() const;

private:
    int m_Level{100};
    bool m_Charging{false};
};
} // namespace flachead::models
