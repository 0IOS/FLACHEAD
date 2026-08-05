#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace flachead::services
{
struct Notification
{
    std::string id;
    std::string title;
    std::string body;
    int kind{0};
    std::int64_t timestampMs{0};
};

// Queues short-lived notifications and hands them to a toast hook on the main
// thread via Update(). Keeps a bounded history for the notification center.
// The hook is intentionally injected (mirroring PlaybackController's volume
// store) so the manager never depends on UI types.
class NotificationManager
{
public:
    using ToastHook = std::function<void(const Notification&)>;

    void SetToastHook(ToastHook hook) { m_ToastHook = std::move(hook); }

    // Enqueues a notification. Reusing an id replaces the pending entry.
    void Push(std::string title, std::string body, int kind = 0, std::string id = {});
    void Dismiss(std::string_view id);
    void DismissAll();

    // Drains the pending queue into the toast hook. Call from the main thread.
    void Update();

    const std::vector<Notification>& History() const { return m_History; }
    std::size_t PendingCount() const { return m_Pending.size(); }

private:
    ToastHook m_ToastHook;
    std::vector<Notification> m_Pending;
    std::vector<Notification> m_History;
    std::int64_t m_NextId{1};
};
} // namespace flachead::services
