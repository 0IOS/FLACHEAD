#include "OverlayManager.hpp"

#include "../Canvas.hpp"

#include <algorithm>

namespace flachead::ui::overlay
{
namespace
{
int LayerRank(Layer layer)
{
    return static_cast<int>(layer);
}
} // namespace

void OverlayManager::Push(std::string id, std::shared_ptr<Widget> widget, OverlayKind kind, Layer layer)
{
    Dismiss(id);

    Entry entry;
    entry.entry.id = std::move(id);
    entry.entry.kind = kind;
    entry.entry.layer = layer;
    entry.entry.widget = std::move(widget);
    entry.lifetime = kind == OverlayKind::Toast ? m_ToastLifetime : -1.0f;
    m_Entries.push_back(std::move(entry));
}

void OverlayManager::PushToast(std::string id, std::shared_ptr<Widget> widget, Layer layer)
{
    Push(std::move(id), std::move(widget), OverlayKind::Toast, layer);
}

void OverlayManager::PushPanel(std::string id, std::shared_ptr<Widget> widget, Layer layer)
{
    Push(std::move(id), std::move(widget), OverlayKind::Panel, layer);
}

void OverlayManager::Dismiss(std::string_view id)
{
    for (auto& entry : m_Entries)
    {
        if (entry.entry.id == id && !entry.removing)
        {
            entry.removing = true;
            entry.removalElapsed = 0.0f;
        }
    }
}

void OverlayManager::DismissAll()
{
    for (auto& entry : m_Entries)
    {
        entry.removing = true;
        entry.removalElapsed = 0.0f;
    }
}

bool OverlayManager::Has(std::string_view id) const
{
    for (const auto& entry : m_Entries)
    {
        if (entry.entry.id == id)
        {
            return true;
        }
    }
    return false;
}

void OverlayManager::Update(float deltaSeconds)
{
    for (auto& entry : m_Entries)
    {
        entry.elapsed += deltaSeconds;

        if (!entry.removing && entry.lifetime > 0.0f && entry.elapsed >= entry.lifetime)
        {
            entry.removing = true;
            entry.removalElapsed = 0.0f;
        }

        if (entry.removing)
        {
            entry.removalElapsed += deltaSeconds;
            const float t = std::clamp(entry.removalElapsed / m_FadeOutTime, 0.0f, 1.0f);
            entry.entry.alpha = 1.0f - t;
        }
        else
        {
            const float t = std::clamp(entry.elapsed / m_FadeInTime, 0.0f, 1.0f);
            entry.entry.alpha = t;
        }

        if (entry.entry.widget)
        {
            entry.entry.widget->SetOpacity(entry.entry.alpha);
            entry.entry.widget->Update(deltaSeconds);
        }
    }
    RemoveFinished();
}

void OverlayManager::RemoveFinished()
{
    m_Entries.erase(std::remove_if(m_Entries.begin(), m_Entries.end(),
                                   [this](const Entry& entry) {
                                       return entry.removing && entry.removalElapsed >= m_FadeOutTime;
                                   }),
                    m_Entries.end());
}

void OverlayManager::Draw(Canvas& canvas)
{
    std::vector<std::size_t> order;
    order.reserve(m_Entries.size());
    for (std::size_t i = 0; i < m_Entries.size(); ++i)
    {
        order.push_back(i);
    }
    std::stable_sort(order.begin(), order.end(),
                     [this](std::size_t a, std::size_t b) {
                         return LayerRank(m_Entries[a].entry.layer) < LayerRank(m_Entries[b].entry.layer);
                     });

    for (const std::size_t index : order)
    {
        const auto& entry = m_Entries[index];
        if (!entry.entry.widget || entry.entry.alpha <= 0.0f)
        {
            continue;
        }
        entry.entry.widget->Draw(canvas);
    }
}
} // namespace flachead::ui::overlay
