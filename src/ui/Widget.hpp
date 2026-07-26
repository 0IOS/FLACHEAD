#pragma once

#include <string>
#include <string_view>

#include "../math/Rect.hpp"

namespace flachead::ui
{

class Canvas;

class Widget
{
public:
    virtual ~Widget() = default;

    // Existing API
    virtual void SetId(std::string_view id);
    virtual std::string_view Id() const;

    virtual void SetEnabled(bool enabled);
    virtual bool Enabled() const;

    // ---------- NEW ----------

    virtual void Draw(Canvas&) {}

    virtual void Update(float) {}

    virtual void Layout() {}

    void SetBounds(const Rect& bounds);
    const Rect& Bounds() const;

    void SetVisible(bool visible);
    bool Visible() const;

protected:
    std::string m_Id;

    bool m_Enabled{true};

    bool m_Visible{true};

    Rect m_Bounds;
};

}