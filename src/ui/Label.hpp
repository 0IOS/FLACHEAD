#pragma once

#include "Widget.hpp"

#include <string>
#include <string_view>

namespace flachead::ui
{
class Label : public Widget
{
public:
    void SetText(std::string_view text);
    std::string_view Text() const;

    void Draw(Canvas& canvas) override;

private:
    std::string m_Text;
};
} // namespace flachead::ui
