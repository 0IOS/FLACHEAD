#pragma once

#include "../input/InputEvent.hpp"
#include "../layout/LayoutEngine.hpp"
#include "../math/Rect.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace flachead::ui
{
class Canvas;

// Base class of every UI control. A Widget is a positioned, drawable element
// that can own children, a layout, and interaction state. Widgets compose
// into trees: containers draw, update, lay out and hit-test their children.
//
// Interaction model: every state change (focused, pressed, selected,
// hovered, disabled) is explicit so the Focus Engine and animations can react
// to it uniformly. Draw() draws children first, then OnDraw for the widget
// itself; leaf widgets override OnDraw.
class Widget
{
public:
    Widget();
    virtual ~Widget() = default;

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    // Identity / visibility / rendering.
    void SetId(std::string_view id) { m_Id = std::string{id}; }
    std::string_view Id() const { return m_Id; }
    void SetEnabled(bool enabled);
    bool Enabled() const { return m_Enabled; }
    void SetVisible(bool visible) { m_Visible = visible; }
    bool Visible() const { return m_Visible; }
    void SetOpacity(float opacity) { m_Opacity = opacity; }
    float Opacity() const { return m_Opacity; }

    // Geometry.
    void SetBounds(const Rect& bounds) { m_Bounds = bounds; }
    const Rect& Bounds() const { return m_Bounds; }
    Rect GlobalBounds() const;
    bool ContainsPoint(const Vec2& point) const;

    // Interaction state.
    void SetFocusable(bool focusable) { m_Focusable = focusable; }
    bool Focusable() const { return m_Focusable; }
    bool Focused() const { return m_Focused; }
    bool Pressed() const { return m_Pressed; }
    void SetSelected(bool selected);
    bool Selected() const { return m_Selected; }
    bool Hovered() const { return m_Hovered; }
    bool Disabled() const { return !m_Enabled; }

    // Tree.
    Widget* Parent() const { return m_Parent; }
    void AddChild(std::unique_ptr<Widget> child);
    void RemoveChild(std::string_view id);
    void RemoveAllChildren();
    const std::vector<std::unique_ptr<Widget>>& Children() const { return m_Children; }
    Widget* FindById(std::string_view id);
    const Widget* FindById(std::string_view id) const;

    // Layout. PreferredSize reports the natural size; Layout arranges
    // children inside bounds (using the widget's Layout, when set).
    virtual Vec2 PreferredSize() const { return m_Bounds.size; }
    virtual void Layout(const Rect& bounds);
    void SetLayout(std::unique_ptr<flachead::layout::LayoutEngine> layout)
    {
        m_Layout = std::move(layout);
    }
    flachead::layout::LayoutEngine* LayoutValue() const { return m_Layout.get(); }

    // Lifecycle hooks.
    virtual void Update(float deltaSeconds);
    virtual void Draw(Canvas& canvas);
    virtual bool HandleEvent(const flachead::input::InputEvent& event);

    // Called by the Focus Engine / input routing.
    void NotifyFocusGained();
    void NotifyFocusLost();
    void SetPressed(bool pressed);
    void SetHovered(bool hovered);

    // Hit testing. Returns the topmost visible child under `point`, or null.
    Widget* ChildAtPoint(const Vec2& point);

protected:
    virtual void OnFocusGained() {}
    virtual void OnFocusLost() {}
    virtual void OnUpdate(float deltaSeconds) { (void)deltaSeconds; }
    virtual void OnDraw(Canvas& canvas) { (void)canvas; }
    virtual bool OnHandleEvent(const flachead::input::InputEvent& event) { (void)event; return false; }

protected:
    std::string m_Id;
    bool m_Enabled{true};
    bool m_Visible{true};
    float m_Opacity{1.0f};
    Rect m_Bounds;

    bool m_Focusable{false};
    bool m_Focused{false};
    bool m_Pressed{false};
    bool m_Selected{false};
    bool m_Hovered{false};

    Widget* m_Parent{nullptr};
    std::vector<std::unique_ptr<Widget>> m_Children;
    std::unique_ptr<flachead::layout::LayoutEngine> m_Layout;
};
} // namespace flachead::ui
