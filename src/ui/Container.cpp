#include "Container.hpp"

#include <algorithm>

namespace flachead::ui
{
void Container::AddChild(std::unique_ptr<Widget> child)
{
    m_Children.push_back(std::move(child));
}

void Container::RemoveChild(std::string_view id)
{
    auto it = std::find_if(m_Children.begin(), m_Children.end(), [&](const auto& child) {
        return child->Id() == id;
    });

    if (it != m_Children.end())
    {
        m_Children.erase(it);
    }
}

std::vector<Widget*> Container::Children() const
{
    std::vector<Widget*> result;
    result.reserve(m_Children.size());
    for (const auto& child : m_Children)
    {
        result.push_back(child.get());
    }
    return result;
}
} // namespace flachead::ui
