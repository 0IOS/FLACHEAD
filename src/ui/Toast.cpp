#include "Toast.hpp"

#include "Canvas.hpp"

#include <algorithm>
#include <utility>

namespace flachead::ui
{
Toast::Toast()
{
    SetFocusable(false);
}

void Toast::SetTitle(std::string_view title)
{
    m_Title = title;
}

void Toast::SetMessage(std::string_view message)
{
    m_Message = message;
}

Vec2 Toast::PreferredSize() const
{
    const float width = std::max(180.0f, static_cast<float>(m_Message.size()) * 11.0f + 48.0f);
    const float height = m_Title.empty() ? 40.0f : 56.0f;
    return Vec2{width, height};
}

void Toast::OnDraw(Canvas& canvas)
{
    const uint8_t alpha = static_cast<uint8_t>(Opacity() * 255.0f);
    const Color panel{22, 26, 36, alpha};
    const Rect bounds = Bounds();
    const float radius = bounds.size.y * 0.5f;
    canvas.FillRoundedRect(bounds, radius, panel);
    canvas.FillRoundedRect(Rect{bounds.position.x, bounds.position.y, 4.0f, bounds.size.y},
                           radius, Color{m_Accent.r, m_Accent.g, m_Accent.b, alpha});

    const Color titleColor{238, 241, 248, alpha};
    const Color messageColor{150, 158, 175, alpha};
    const float x = bounds.position.x + 16.0f;
    if (m_Title.empty())
    {
        canvas.DrawText(Rect{x, bounds.position.y, bounds.size.x - 28.0f, bounds.size.y},
                        m_Message, messageColor, 16.0f);
    }
    else
    {
        canvas.DrawText(Rect{x, bounds.position.y + 6.0f, bounds.size.x - 28.0f, 22.0f},
                        m_Title, titleColor, 16.0f);
        canvas.DrawText(Rect{x, bounds.position.y + 28.0f, bounds.size.x - 28.0f, 22.0f},
                        m_Message, messageColor, 14.0f);
    }
}
} // namespace flachead::ui
