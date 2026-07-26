#pragma once

#include "Button.hpp"

namespace flachead::ui
{
class IconButton : public Button
{
public:
    explicit IconButton(std::string_view iconName = "");
    void SetIcon(std::string_view iconName);
    std::string_view Icon() const;

private:
    std::string m_IconName;
};
} // namespace flachead::ui
