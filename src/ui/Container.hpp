#pragma once

#include "Widget.hpp"

namespace flachead::ui
{
// A plain container; children are managed by the Widget base and drawn /
// updated / laid out automatically by the base class.
class Container : public Widget
{
public:
    Container() = default;
};
} // namespace flachead::ui
