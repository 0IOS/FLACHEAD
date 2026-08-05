#include "CommandCenter.hpp"

#include <algorithm>

namespace flachead::commands
{
int CommandCenter::Register(Handler handler, int priority)
{
    Entry entry;
    entry.id = m_NextId++;
    entry.priority = priority;
    entry.handler = std::move(handler);
    m_Handlers.push_back(std::move(entry));
    std::stable_sort(m_Handlers.begin(), m_Handlers.end(),
                     [](const Entry& a, const Entry& b) { return a.priority > b.priority; });
    return entry.id;
}

void CommandCenter::Unregister(int subscriptionId)
{
    m_Handlers.erase(std::remove_if(m_Handlers.begin(), m_Handlers.end(),
                                    [subscriptionId](const Entry& e) {
                                        return e.id == subscriptionId;
                                    }),
                     m_Handlers.end());
}

bool CommandCenter::Dispatch(Command command) const
{
    if (command == Command::None)
    {
        return false;
    }
    for (const auto& entry : m_Handlers)
    {
        if (entry.handler && entry.handler(command))
        {
            return true;
        }
    }
    return false;
}
} // namespace flachead::commands
