#include "NotificationManager.hpp"

#include <algorithm>
#include <chrono>
#include <utility>

namespace flachead::services
{
namespace
{
constexpr std::size_t kHistoryCap = 64;
} // namespace

void NotificationManager::Push(std::string title, std::string body, int kind, std::string id)
{
    Notification notification;
    notification.id = id.empty() ? "notif_" + std::to_string(m_NextId++) : std::move(id);
    notification.title = std::move(title);
    notification.body = std::move(body);
    notification.kind = kind;
    notification.timestampMs = static_cast<std::int64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch())
            .count());

    auto pending = std::find_if(m_Pending.begin(), m_Pending.end(),
                                [&](const Notification& entry) { return entry.id == notification.id; });
    if (pending != m_Pending.end())
    {
        *pending = notification;
    }
    else
    {
        m_Pending.push_back(notification);
    }

    m_History.push_back(notification);
    if (m_History.size() > kHistoryCap)
    {
        m_History.erase(m_History.begin(),
                        m_History.begin() + static_cast<std::ptrdiff_t>(m_History.size() - kHistoryCap));
    }
}

void NotificationManager::Dismiss(std::string_view id)
{
    m_Pending.erase(std::remove_if(m_Pending.begin(), m_Pending.end(),
                                   [id](const Notification& entry) { return entry.id == id; }),
                    m_Pending.end());
}

void NotificationManager::DismissAll()
{
    m_Pending.clear();
}

void NotificationManager::Update()
{
    if (m_Pending.empty() || !m_ToastHook)
    {
        return;
    }

    std::vector<Notification> ready;
    ready.swap(m_Pending);
    for (const auto& notification : ready)
    {
        m_ToastHook(notification);
    }
}
} // namespace flachead::services
