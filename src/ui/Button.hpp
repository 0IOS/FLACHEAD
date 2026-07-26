#pragma once

#include "Widget.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace flachead::ui
{
class Button : public Widget
{
public:
    using ClickHandler = std::function<void()>;

    void SetText(std::string_view text);
    std::string_view Text() const;
    void SetClickHandler(ClickHandler handler);
    void Click();

    void Draw(Canvas& canvas) override;

private:
    std::string m_Text;
    ClickHandler m_ClickHandler;
};
} // namespace flachead::ui
