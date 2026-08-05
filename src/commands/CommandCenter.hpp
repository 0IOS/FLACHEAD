#pragma once

#include "Command.hpp"

#include <functional>
#include <utility>
#include <vector>

namespace flachead::commands
{
// Routes commands to handlers. Handlers are consulted by registration order
// (highest priority first); the first handler that returns true claims the
// command. Screens register themselves on enter and unregister on exit, so
// the focused screen always wins over the app shell.
class CommandCenter
{
public:
    using Handler = std::function<bool(Command)>;

    // Returns a subscription id used to unregister.
    int Register(Handler handler, int priority = 0);
    void Unregister(int subscriptionId);

    bool Dispatch(Command command) const;

    int HandlerCount() const { return static_cast<int>(m_Handlers.size()); }

private:
    struct Entry
    {
        int id;
        int priority;
        Handler handler;
    };

    std::vector<Entry> m_Handlers;
    int m_NextId{1};
};
} // namespace flachead::commands
