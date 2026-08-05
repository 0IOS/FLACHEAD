#pragma once

#include "Widget.hpp"

#include "../math/Color.hpp"

#include <string>
#include <string_view>

namespace flachead::ui
{
// A transient notification pill. The OverlayManager controls its lifetime and
// animation; the widget draws the pill with an optional title + message.
class Toast : public Widget
{
public:
    Toast();

    void SetTitle(std::string_view title);
    std::string_view Title() const { return m_Title; }
    void SetMessage(std::string_view message);
    void SetAccent(const Color& color) { m_Accent = color; }

    Vec2 PreferredSize() const override;

protected:
    void OnDraw(Canvas& canvas) override;

private:
    std::string m_Title;
    std::string m_Message;
    Color m_Accent{124, 58, 237};
};
} // namespace flachead::ui
