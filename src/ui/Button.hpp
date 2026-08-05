#pragma once

#include "Widget.hpp"

#include "../math/Color.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace flachead::ui
{
// A tappable button with focused / pressed / selected / disabled visual
// states and a subtle press animation. Emits its click handler on tap or
// when Select is activated while focused.
class Button : public Widget
{
public:
    using ClickHandler = std::function<void()>;

    Button();

    void SetText(std::string_view text);
    std::string_view Text() const { return m_Text; }

    void SetClickHandler(ClickHandler handler) { m_ClickHandler = std::move(handler); }
    void Click();

    void SetAccentColor(const Color& color) { m_Accent = color; }
    const Color& AccentColor() const { return m_Accent; }

    void SetBackground(const Color& color) { m_Background = color; }
    const Color& Background() const { return m_Background; }

    Vec2 PreferredSize() const override;
    void SetPressCallback(std::function<void(bool)> callback)
    {
        m_PressCallback = std::move(callback);
    }

protected:
    void OnUpdate(float deltaSeconds) override;
    void OnDraw(Canvas& canvas) override;
    bool OnHandleEvent(const flachead::input::InputEvent& event) override;
    void OnFocusGained() override;
    void OnFocusLost() override;

private:
    std::string m_Text;
    ClickHandler m_ClickHandler;
    std::function<void(bool)> m_PressCallback;
    Color m_Accent{124, 58, 237};
    Color m_Background{20, 24, 34};
    float m_PressAmount{0.0f};
    bool m_PressedInside{false};
};
} // namespace flachead::ui
