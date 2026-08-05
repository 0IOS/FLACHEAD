#include "EventBus.hpp"

namespace flachead::events
{
int EventBus::Subscribe(Type type, Handler handler)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    const int id = m_NextId++;
    m_Subscribers[type].push_back(Subscription{id, std::move(handler)});
    return id;
}

void EventBus::Unsubscribe(int subscriptionId)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    for (auto& [type, subscriptions] : m_Subscribers)
    {
        for (auto it = subscriptions.begin(); it != subscriptions.end(); ++it)
        {
            if (it->id == subscriptionId)
            {
                subscriptions.erase(it);
                return;
            }
        }
    }
}

void EventBus::Publish(const Event& event)
{
    std::vector<Subscription> subscriptions;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        const auto it = m_Subscribers.find(event.type);
        if (it == m_Subscribers.end())
        {
            return;
        }
        // Copy the list so handlers that subscribe/unsubscribe during dispatch
        // do not invalidate the iterator, and so handlers that publish re-
        // entrantly (nested events) do not deadlock on this mutex.
        subscriptions = it->second;
    }

    for (const auto& subscription : subscriptions)
    {
        if (subscription.handler)
        {
            subscription.handler(event);
        }
    }
}

void EventBus::Publish(Type type)
{
    Publish(Event{type});
}

void EventBus::Publish(Type type, int64_t intValue)
{
    Publish(Event{type, intValue, 0.0, {}});
}
} // namespace flachead::events
