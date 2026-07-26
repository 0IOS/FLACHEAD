#pragma once

#include <string>
#include <string_view>

namespace flachead::models
{
class SettingsModel
{
public:
    SettingsModel() = default;

    void SetTheme(std::string_view theme);
    void SetVolume(int volume);
    void SetBrightness(int brightness);

    std::string_view Theme() const;
    int Volume() const;
    int Brightness() const;

private:
    std::string m_Theme{"default"};
    int m_Volume{50};
    int m_Brightness{60};
};
} // namespace flachead::models
