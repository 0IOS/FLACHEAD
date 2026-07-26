#include "View.hpp"

#include "Canvas.hpp"

namespace flachead::ui
{

void View::AddChild(std::unique_ptr<View> child)
{
    m_Children.push_back(std::move(child));
}

void View::Draw(Canvas& canvas)
{
    OnDraw(canvas);

    for(auto& child : m_Children)
    {
        child->Draw(canvas);
    }
}

void View::SetBounds(const Rect& rect)
{
    m_Bounds = rect;
}

const Rect& View::Bounds() const
{
    return m_Bounds;
}

void View::OnDraw(Canvas&)
{
}

}