#pragma once

#include "../commands/Command.hpp"
#include "../math/Rect.hpp"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace flachead::focus
{
// Geometry-based focus navigation. Focusable controls register a rect; the
// manager moves focus along the nearest neighbour in the requested direction.
// Handles both d-pad navigation and touch-tap-to-focus.
class FocusManager
{
public:
    using ChangeCallback = std::function<void(std::string_view)>;

    void Register(std::string id, const Rect& rect);
    void Unregister(std::string_view id);
    void SetRect(std::string_view id, const Rect& rect);

    void SetFocus(std::string_view id);
    bool Move(commands::Command direction);
    bool HasFocus(std::string_view id) const { return m_FocusedId == id; }

    std::string_view FocusedId() const { return m_FocusedId; }
    bool HasFocusable() const { return !m_Nodes.empty(); }

    void SetChangeCallback(ChangeCallback callback) { m_OnChange = std::move(callback); }
    void Clear();

private:
    bool MoveDirection(float dx, float dy);

    struct Node
    {
        Rect rect;
    };

    std::unordered_map<std::string, Node> m_Nodes;
    std::string m_FocusedId;
    ChangeCallback m_OnChange;
};
} // namespace flachead::focus
