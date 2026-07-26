#include "SettingsModel.hpp"

namespace flachead::models
{
void SettingsModel::SetTheme(std::string_view theme)
{
    m_Theme = theme;
}

void SettingsModel::SetVolume(int volume)
{
    m_Volume = volume;
}

void SettingsModel::SetBrightness(int brightness)
{
    m_Brightness = brightness;
}

std::string_view SettingsModel::Theme() const
{
    return m_Theme;
}

int SettingsModel::Volume() const
{
    return m_Volume;
}

int SettingsModel::Brightness() const
{
    return m_Brightness;
}
} // namespace flachead::models
