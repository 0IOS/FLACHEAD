#include "Label.hpp"

#include "Canvas.hpp"
#include "../math/Color.hpp"

namespace flachead::ui
{
void Label::SetText(std::string_view text)
{
    m_Text = text;
}

std::string_view Label::Text() const
{
    return m_Text;
}

void Label::Draw(Canvas& canvas)
{
    if (!Visible())
    {
        return;
    }

    canvas.DrawText(Bounds(), m_Text, Color::White, 18.0f);
}
} // namespace flachead::ui
