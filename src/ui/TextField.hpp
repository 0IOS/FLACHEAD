#pragma once

#include "Widget.hpp"

#include "../math/Color.hpp"

#include <functional>
#include <string>
#include <string_view>

namespace flachead::ui
{
// A single-line text entry. Appends printable characters and removes them on
// backspace when it receives key events while focused. Draws a rounded field
// with a placeholder when empty. Used by the universal search and settings.
class TextField : public Widget
{
public:
    TextField();

    void SetQuery(std::string_view query);
    std::string_view Query() const { return m_Query; }

    void SetPlaceholder(std::string_view placeholder) { m_Placeholder = std::string{placeholder}; }
    void SetPlaceholderColor(const Color& color) { m_PlaceholderColor = color; }
    void SetBackground(const Color& color) { m_Background = color; }
    void SetColor(const Color& color) { m_Color = color; }

    Vec2 PreferredSize() const override;

    using ChangeHandler = std::function<void()>;
    void SetChangeHandler(ChangeHandler handler) { m_ChangeHandler = std::move(handler); }

    using SubmitHandler = std::function<void()>;
    void SetSubmitHandler(SubmitHandler handler) { m_SubmitHandler = std::move(handler); }

protected:
    void OnDraw(Canvas& canvas) override;
    bool OnHandleEvent(const flachead::input::InputEvent& event) override;
    void OnFocusGained() override;
    void OnFocusLost() override;

private:
    void Append(char ch);
    void Backspace();

    std::string m_Query;
    std::string m_Placeholder;
    Color m_PlaceholderColor{100, 116, 139};
    Color m_Background{20, 24, 36};
    Color m_Color{241, 245, 249};
    ChangeHandler m_ChangeHandler;
    SubmitHandler m_SubmitHandler;
};
} // namespace flachead::ui
