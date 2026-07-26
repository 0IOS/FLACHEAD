#pragma once

#include <memory>
#include <vector>

#include "../math/Rect.hpp"

namespace flachead::ui
{

class Canvas;

class View
{
public:
    virtual ~View() = default;

    void AddChild(std::unique_ptr<View> child);

    void Draw(Canvas& canvas);

    void SetBounds(const Rect& rect);

    const Rect& Bounds() const;

protected:
    virtual void OnDraw(Canvas& canvas);

private:
    Rect m_Bounds;

    std::vector<std::unique_ptr<View>> m_Children;
};

}