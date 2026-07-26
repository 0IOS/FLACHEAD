#pragma once

#include "Widget.hpp"

#include <memory>
#include <vector>

namespace flachead::ui
{
class Container : public Widget
{
public:
    void AddChild(std::unique_ptr<Widget> child);
    void RemoveChild(std::string_view id);
    std::vector<Widget*> Children() const;

private:
    std::vector<std::unique_ptr<Widget>> m_Children;
};
} // namespace flachead::ui
