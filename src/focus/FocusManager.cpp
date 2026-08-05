#include "FocusManager.hpp"

#include <cmath>
#include <limits>

namespace flachead::focus
{
void FocusManager::Register(std::string id, const Rect& rect)
{
    m_Nodes[std::move(id)] = Node{rect};
}

void FocusManager::Unregister(std::string_view id)
{
    m_Nodes.erase(std::string{id});
    if (m_FocusedId == id)
    {
        m_FocusedId.clear();
    }
}

void FocusManager::SetRect(std::string_view id, const Rect& rect)
{
    auto it = m_Nodes.find(std::string{id});
    if (it != m_Nodes.end())
    {
        it->second.rect = rect;
    }
}

void FocusManager::SetFocus(std::string_view id)
{
    if (m_Nodes.find(std::string{id}) == m_Nodes.end() || m_FocusedId == id)
    {
        return;
    }
    m_FocusedId = std::string{id};
    if (m_OnChange)
    {
        m_OnChange(m_FocusedId);
    }
}

void FocusManager::Clear()
{
    m_Nodes.clear();
    m_FocusedId.clear();
}

bool FocusManager::MoveDirection(float dx, float dy)
{
    if (m_Nodes.empty())
    {
        return false;
    }

    const std::string current = m_FocusedId;
    Rect currentRect;
    bool hasCurrent = false;
    if (!current.empty())
    {
        auto it = m_Nodes.find(current);
        if (it != m_Nodes.end())
        {
            currentRect = it->second.rect;
            hasCurrent = true;
        }
    }

    // Center of the current node; fall back to the center of the screen
    // union when nothing is focused yet.
    Vec2 origin;
    if (hasCurrent)
    {
        origin = Vec2{currentRect.position.x + currentRect.size.x * 0.5f,
                      currentRect.position.y + currentRect.size.y * 0.5f};
    }
    else
    {
        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float maxY = std::numeric_limits<float>::lowest();
        for (const auto& entry : m_Nodes)
        {
            minX = std::min(minX, entry.second.rect.position.x);
            minY = std::min(minY, entry.second.rect.position.y);
            maxX = std::max(maxX, entry.second.rect.position.x + entry.second.rect.size.x);
            maxY = std::max(maxY, entry.second.rect.position.y + entry.second.rect.size.y);
        }
        origin = Vec2{(minX + maxX) * 0.5f, (minY + maxY) * 0.5f};
    }

    std::string best;
    float bestScore = std::numeric_limits<float>::max();
    for (const auto& entry : m_Nodes)
    {
        if (hasCurrent && entry.first == current)
        {
            continue;
        }
        const Rect& rect = entry.second.rect;
        const Vec2 center{rect.position.x + rect.size.x * 0.5f,
                          rect.position.y + rect.size.y * 0.5f};
        const float cx = center.x - origin.x;
        const float cy = center.y - origin.y;

        const float dot = cx * dx + cy * dy;
        if (dot <= 0.0f)
        {
            continue;
        }
        const float cross = std::abs(cx * dy - cy * dx);
        const float distance = cx * cx + cy * cy;
        // Prefer candidates aligned with the direction; penalize off-axis.
        const float score = distance + cross * cross;
        if (score < bestScore)
        {
            bestScore = score;
            best = entry.first;
        }
    }

    if (best.empty())
    {
        return false;
    }
    SetFocus(best);
    return true;
}

bool FocusManager::Move(commands::Command direction)
{
    switch (direction)
    {
        case commands::Command::Up: return MoveDirection(0.0f, -1.0f);
        case commands::Command::Down: return MoveDirection(0.0f, 1.0f);
        case commands::Command::Left: return MoveDirection(-1.0f, 0.0f);
        case commands::Command::Right: return MoveDirection(1.0f, 0.0f);
        default: return false;
    }
}
} // namespace flachead::focus
