#pragma once

#include "Widget.hpp"

#include "../math/Color.hpp"

#include <string>
#include <string_view>

namespace flachead::ui
{
// Single-line text. Alignment, color and size are configurable; the widget
// draws nothing when its text is empty.
class Label : public Widget
{
public:
    Label();

    void SetText(std::string_view text);
    std::string_view Text() const { return m_Text; }

    void SetColor(const Color& color) { m_Color = color; }
    const Color& ColorValue() const { return m_Color; }

    void SetFontSize(float size) { m_FontSize = size; }
    float FontSize() const { return m_FontSize; }

    enum class Align
    {
        Left,
        Center,
        Right,
    };
    void SetAlign(Align align) { m_Align = align; }
    Align AlignValue() const { return m_Align; }

    Vec2 PreferredSize() const override;

protected:
    void OnDraw(Canvas& canvas) override;

private:
    std::string m_Text;
    Color m_Color{255, 255, 255};
    float m_FontSize{18.0f};
    Align m_Align{Align::Left};
};
} // namespace flachead::ui
